#include <utils/cmd_parse.hh>
#include <stdexcept>
#include <sstream>
#include <iostream>

using namespace utils;
using Result = CmdParser::Result;

Result::Result(int argc, char const* argv[], const ArgSpec& spec) {
    enum class State {
        POSITIONAL,
        NAMED,
        REST,
    } state = State::POSITIONAL;

    int pos = 0;
    _is_help = false;

    const ArgInfo * named_info = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (state == State::REST) {
            _rest.push_back(argv[i]);
            continue;
        }

        auto current = std::string(argv[i]);

        if (current.rfind("--", 0) == 0) {
            if (state != State::POSITIONAL) {
                throw std::runtime_error("Unexpected argument: " + current);
            }

            auto name = current.substr(2);

            if (name == spec.help.first) {
                _is_help = true;
                return;
            }

            auto it = spec.named.find(name);

            if (it == spec.named.end()) {
                throw std::runtime_error("Unknown argument: " + current);
            }

            named_info = &it->second;

            if (named_info->type != ArgType::NONE) {
                state = State::NAMED;
            }
            else {
                _args[named_info->name] = Arg(ArgType::NONE);
            }

            continue;
        }

        if (current.rfind('-', 0) == 0) {
            if (state != State::POSITIONAL) {
                throw std::runtime_error("Unexpected argument: " + current);
            }

            auto sh = current.substr(1);

            if (sh == spec.help.second) {
                _is_help = true;
                return;
            }

            auto it = spec.shorthands.find(sh);

            if (it == spec.shorthands.end()) {
                throw std::runtime_error("Unknown argument: " + current);
            }

            named_info = &it->second;

            if (named_info->type != ArgType::NONE) {
                state = State::NAMED;
            }
            else {
                _args[named_info->name] = Arg(ArgType::NONE);
            }

            continue;
        }

        switch (state) {
            case State::POSITIONAL: {
                auto size = spec.positional.size();
                const ArgInfo * arg = nullptr;

                while (pos < size) {
                    arg = &spec.positional[pos];

                    // Argumentos posicionais podem ser omitidos se já tiverem
                    // sido especificados como argumentos nomeados
                    if (_args.find(arg->name) == _args.end()) {
                        break;
                    }

                    ++pos;
                }

                if (pos >= size) {
                    _rest.push_back(argv[i]);
                    state = State::REST;
                    break;
                }

                _args[arg->name] = Arg(arg->type, argv[i]);
                break;
            }
            case State::NAMED: {
                state = State::POSITIONAL;

                if (_args.find(named_info->name) != _args.end()) {
                    throw std::runtime_error("Duplicate argument: " + current);
                }

                _args[named_info->name] = Arg(named_info->type, argv[i]);
                break;
            }
        }
    }

    for (auto& [name, info] : spec.named) {
        if (_args.find(name) == _args.end()) {
            if (info.required) {
                throw std::runtime_error("Missing required argument: " + info.name);
            }

            if (info.type != ArgType::NONE) {
                _args[name] = info.default_value;
            }
        }
    }
}

Arg Result::get(std::string name) {
    auto it = _args.find(name);

    if (it == _args.end()) {
        return Arg(ArgType::ABSCENT);
    }

    return it->second;
}

bool Result::has(std::string name) {
    return _args.find(name) != _args.end();
}

Arg Result::operator[](std::string name) {
    return get(name);
}

CmdParser::CmdParser() {}

CmdParser::Result CmdParser::parse(int argc, char const * argv[]) const {
    try {
        auto result = Result(argc, argv, _spec);

        if (result.is_help()) {
            std::cout << help(argv[0]) << std::endl;
            exit(0);
        }

        return result;
    }
    catch (std::runtime_error e) {
        std::stringstream ss;
        ss  << e.what() << "\n\n"
            << help(argv[0]);

        throw std::runtime_error(ss.str());
    }
}

CmdParser & CmdParser::add_arg(const ArgInfo& info) {
    if (info.type == ArgType::ABSCENT) {
        throw std::runtime_error("Invalid argument type");
    }

    if (info.positional) {
        if (
            _spec.positional.size() &&
            !_spec.positional.back().required &&
            info.required
        ) {
            throw std::runtime_error(
                "Required positional argument after optional: " + info.name
            );
        }

        _spec.positional.push_back(info);
    }

    if (info.required && info.default_value.type() != ArgType::ABSCENT) {
        throw std::runtime_error(
            "Required argument with default value: " + info.name
        );
    }

    _add_to_map(info.name, info);

    if (!info.shorthand.empty()) {
        _add_to_map(info.shorthand, info, true);
    }

    return *this;
}

CmdParser& CmdParser::add_help_arg(const std::string& name, const std::string& shorthand) {
    _spec.help = { name, shorthand };

    add_arg({
        .name = name,
        .type = ArgType::NONE,
        .required = false,
        .positional = false,
        .description = "Print this help message and exit",
        .shorthand = shorthand,
    });

    return *this;
}

std::string CmdParser::help(std::string name) const {
    std::stringstream ss;

    ss << "Usage: " << name;

    for (auto &arg: _spec.positional) {
        if (arg.required) {
            ss << " <" << arg.name << ">";
        } else {
            ss << " [" << arg.name << "]";
        }

        ss << " ";
    }

    for (auto &[name, arg]: _spec.named) {
        if (arg.positional) {
            continue;
        }

        if (!arg.required) {
            ss << "[";
        }

        ss << "--" << name;

        if (!arg.shorthand.empty()) {
            ss << " (-" << arg.shorthand << ")";
        }

        if (arg.type != ArgType::NONE) {
            ss << " <" << arg.name << ">";
        }

        if (!arg.required) {
            ss << "]";
        }

        ss << " ";
    }

    ss << "...\n\nParameters:\n";

    for (auto &arg: _spec.positional) {
        ss << "  " << arg.name << " ( --" << arg.name;

        if (!arg.shorthand.empty()) {
            ss << ", -" << arg.shorthand;
        }

        ss << " ): " << arg.description << "\n";
    }

    if (_spec.positional.size() && _spec.named.size()) {
        ss << "\n";
    }

    for (auto& [name, arg] : _spec.named) {
        if (arg.positional) continue;

        ss << "  --" << name;

        if (!arg.shorthand.empty()) {
            ss << ", -" << arg.shorthand;
        }

        ss << ":  " << arg.description << "\n";
    }

    return ss.str();
}

void CmdParser::_add_to_map(std::string name, const ArgInfo& info, bool shorthand) {
    auto & map = shorthand ? _spec.shorthands : _spec.named;

    if (map.find(name) != map.end()) {
        throw std::runtime_error("Duplicate argument name: " + name);
    }

    map[name] = info;
}

Arg::Arg(ArgType type, char const * value): _type(type) {
    switch (type) {
        case ArgType::STRING:
            _value._string = value;
            break;
        case ArgType::INT:
            _value._int = std::stoi(value);
            break;
        case ArgType::FLOAT:
            _value._float = std::stod(value);
            break;
    }
}

Arg::Arg(char const * value) {
    _type = ArgType::STRING;
    _value._string = value;
}

Arg::Arg(int value) {
    _type = ArgType::INT;
    _value._int = value;
}

Arg::Arg(double value) {
    _type = ArgType::FLOAT;
    _value._float = value;
}

Arg::operator int() const {
    if (_type != ArgType::INT) {
        throw std::runtime_error("Invalid type conversion");
    }

    return _value._int;
}

Arg::operator bool() const {
    if (_type != ArgType::NONE || _type != ArgType::ABSCENT) {
        throw std::runtime_error("Invalid type conversion");
    }

    return _type == ArgType::NONE;
}

Arg::operator double() const {
    if (_type != ArgType::FLOAT) {
        throw std::runtime_error("Invalid type conversion");
    }

    return _value._float;
}

Arg::operator std::string() const {
    if (_type != ArgType::STRING) {
        throw std::runtime_error("Invalid type conversion");
    }

    return std::string(_value._string);
}
