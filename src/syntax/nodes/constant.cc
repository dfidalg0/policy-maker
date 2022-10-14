#include <syntax/nodes/constant.hh>

using namespace syntax;

void Constant::print(uint level) {
    std::cout << indent(level) << "> Constant: \n";
    std::cout << indent(level + 2) << "- Type: " << type_to_string(_type) << std::endl;
    std::cout << indent(level + 2) << "- Value: " << _value << std::endl;
}

std::string Constant::type_to_string(Type type) {
    switch (type) {
        case Type::integer: return "integer";
        case Type::string: return "string";
        case Type::boolean: return "boolean";
        case Type::null: return "null";
        default: return "unknown";
    }
}

Constant::operator int() {
    if (_type == Type::boolean) {
        return (int) (bool) *this;
    }

    if (_type == Type::null) {
        return 0;
    }

    try {
        return std::stoi(_value);
    }
    catch (std::invalid_argument) {
        throw std::runtime_error("Cannot convert string to integer");
    }
}

Constant::operator std::string() {
    if (_type == Type::boolean) {
        return ((bool) *this) ? "true" : "false";
    }

    if (_type == Type::integer) {
        return std::to_string((int) *this);
    }

    if (_type == Type::null) {
        return "null";
    }

    return _value;
}

Constant::operator bool() {
    if (_type == Type::integer) {
        return (bool) (int) *this;
    }

    if (_type == Type::string) {
        return !this->value().empty();
    }

    if (_type == Type::null) {
        return false;
    }

    return _value == "true" || _value == "1";
}

bool Constant::is_truthy() {
    return (bool) *this;
}
