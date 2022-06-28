#ifndef __ERRORS_HH__
#define __ERRORS_HH__

#include <exception>
#include <string>

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

class ParseError : public std::exception {
public:
    ParseError(const std::string file)
        : _msg("Error while parsing file: \"" + file + "\"") {}

    virtual const char *what() const noexcept {
        return _msg.c_str();
    }
private:
    const std::string _msg;
};

#endif // __ERRORS_HH__
