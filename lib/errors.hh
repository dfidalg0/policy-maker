#ifndef __ERRORS_HH__
#define __ERRORS_HH__

#include <exception>
#include <string>
#include <vector>
#include <sstream>
#include <lexicon/position.hh>

class FileNotFoundError : public std::exception {
public:
    FileNotFoundError(const std::string file)
        : _msg("File not found: " + file) {}

    virtual const char *what() const noexcept {
        return _msg.c_str();
    }
private:
    const std::string _msg;
};

class CompilerError: public std::exception {
public:
    CompilerError(const std::string msg)
        : _msg(msg) {}

    virtual const char *what() const noexcept {
        return _msg.c_str();
    }

    const std::string& msg() const {
        return _msg;
    }

    CompilerError& push(const Position pos, std::string desc) {
        _stack.push_back({ pos, desc });
        return *this;
    }

    CompilerError build(std::string filename) {
        std::stringstream ss;

        ss << _msg;

        for (auto& [pos, desc] : _stack) {
            ss << "\n    at " << desc << " (" << filename << ":" << pos << ")";
        }

        return CompilerError(ss.str());
    }
private:
    struct StackFrame {
        Position pos;
        std::string desc;
    };

    const std::string _msg;
    std::vector<StackFrame> _stack;
};

#endif // __ERRORS_HH__
