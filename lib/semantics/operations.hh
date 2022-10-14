#ifndef __SEMANTICS_OPERATIONS_HH__
#define __SEMANTICS_OPERATIONS_HH__

#include <string>

// int-int operators
int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int divide(int a, int b);
int modulo(int a, int b);
int bit_and(int a, int b);
int bit_or(int a, int b);
int bit_xor(int a, int b);
int bit_lsh(int a, int b);
int bit_rsh(int a, int b);
bool equal(int a, int b);
bool not_equal(int a, int b);
bool less_than(int a, int b);
bool less_than_equal(int a, int b);
bool greater_than(int a, int b);
bool greater_than_equal(int a, int b);
bool logical_and(int a, int b);
bool logical_or(int a, int b);

// bool-bool operators
bool add(bool a, bool b);
bool subtract(bool a, bool b);
bool multiply(bool a, bool b);
bool divide(bool a, bool b);
bool modulo(bool a, bool b);
bool bit_and(bool a, bool b);
bool bit_or(bool a, bool b);
bool bit_xor(bool a, bool b);
bool bit_lsh(bool a, bool b);
bool bit_rsh(bool a, bool b);
bool equal(bool a, bool b);
bool not_equal(bool a, bool b);
bool less_than(bool a, bool b);
bool less_than_equal(bool a, bool b);
bool greater_than(bool a, bool b);
bool greater_than_equal(bool a, bool b);
bool logical_and(bool a, bool b);
bool logical_or(bool a, bool b);

// string-string operators
std::string add(std::string a, std::string b);
std::string subtract(std::string a, std::string b);
std::string multiply(std::string a, std::string b);
std::string divide(std::string a, std::string b);
std::string modulo(std::string a, std::string b);
std::string bit_and(std::string a, std::string b);
std::string bit_or(std::string a, std::string b);
std::string bit_xor(std::string a, std::string b);
std::string bit_lsh(std::string a, std::string b);
std::string bit_rsh(std::string a, std::string b);
bool equal(std::string a, std::string b);
bool not_equal(std::string a, std::string b);
bool less_than(std::string a, std::string b);
bool less_than_equal(std::string a, std::string b);
bool greater_than(std::string a, std::string b);
bool greater_than_equal(std::string a, std::string b);
bool logical_and(std::string a, std::string b);
bool logical_or(std::string a, std::string b);

#endif // __SEMANTICS_OPERATIONS_HH__
