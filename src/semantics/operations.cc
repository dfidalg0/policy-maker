#include <semantics/operations.h>
#include <stdexcept>

// int-int operators
int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    return a / b;
}

int modulo(int a, int b) {
    return a % b;
}

int bit_and(int a, int b) {
    return a & b;
}

int bit_or(int a, int b) {
    return a | b;
}

int bit_xor(int a, int b) {
    return a ^ b;
}

int bit_lsh(int a, int b) {
    return a << b;
}

int bit_rsh(int a, int b) {
    return a >> b;
}

bool equal(int a, int b) {
    return a == b;
}

bool not_equal(int a, int b) {
    return a != b;
}

bool less_than(int a, int b) {
    return a < b;
}

bool less_than_equal(int a, int b) {
    return a <= b;
}

bool greater_than(int a, int b) {
    return a > b;
}

bool greater_than_equal(int a, int b) {
    return a >= b;
}

bool logical_and(int a, int b) {
    return a && b;
}

bool logical_or(int a, int b) {
    return a || b;
}

// bool-bool operators
bool add(bool a, bool b) {
    return a + b;
}

bool subtract(bool a, bool b) {
    return a - b;
}

bool multiply(bool a, bool b) {
    return a * b;
}

bool divide(bool a, bool b) {
    return a / b;
}

bool modulo(bool a, bool b) {
    return a % b;
}

bool bit_and(bool a, bool b) {
    return a & b;
}

bool bit_or(bool a, bool b) {
    return a | b;
}

bool bit_xor(bool a, bool b) {
    return a ^ b;
}

bool bit_lsh(bool a, bool b) {
    return a << b;
}

bool bit_rsh(bool a, bool b) {
    return a >> b;
}

bool equal(bool a, bool b) {
    return a == b;
}

bool not_equal(bool a, bool b) {
    return a != b;
}

bool less_than(bool a, bool b) {
    return a < b;
}

bool less_than_equal(bool a, bool b) {
    return a <= b;
}

bool greater_than(bool a, bool b) {
    return a > b;
}

bool greater_than_equal(bool a, bool b) {
    return a >= b;
}

bool logical_and(bool a, bool b) {
    return a && b;
}

bool logical_or(bool a, bool b) {
    return a || b;
}

// string-string operators
std::string add(std::string a, std::string b) {
    return a + b;
}

std::string subtract(std::string a, std::string b) {
    throw std::runtime_error("Cannot subtract strings");
}

std::string multiply(std::string a, std::string b) {
    throw std::runtime_error("Cannot multiply strings");
}

std::string divide(std::string a, std::string b) {
    throw std::runtime_error("Cannot divide strings");
}

std::string modulo(std::string a, std::string b) {
    throw std::runtime_error("Cannot modulo strings");
}

std::string bit_and(std::string a, std::string b) {
    throw std::runtime_error("Cannot bitwise-and strings");
}

std::string bit_or(std::string a, std::string b) {
    throw std::runtime_error("Cannot bitwise-or strings");
}

std::string bit_xor(std::string a, std::string b) {
    throw std::runtime_error("Cannot bitwise-xor strings");
}

std::string bit_lsh(std::string a, std::string b) {
    throw std::runtime_error("Cannot bitwise-left-shift strings");
}

std::string bit_rsh(std::string a, std::string b) {
    throw std::runtime_error("Cannot bitwise-right-shift strings");
}

bool equal(std::string a, std::string b) {
    return a == b;
}

bool not_equal(std::string a, std::string b) {
    return a != b;
}

bool less_than(std::string a, std::string b) {
    return a < b;
}

bool less_than_equal(std::string a, std::string b) {
    return a <= b;
}

bool greater_than(std::string a, std::string b) {
    return a > b;
}

bool greater_than_equal(std::string a, std::string b) {
    return a >= b;
}

bool logical_and(std::string a, std::string b) {
    if (!a.empty()) {
        return !b.empty();
    }

    return false;
}

bool logical_or(std::string a, std::string b) {
    if (a.empty()) {
        return !b.empty();
    }

    return true;
}
