#ifndef __SEMANTICS_OPERATIONS_HH__
#define __SEMANTICS_OPERATIONS_HH__

#include <string>

using str = std::string;

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
str add(str a, str b);
str subtract(str a, str b);
str multiply(str a, str b);
str divide(str a, str b);
str modulo(str a, str b);
str bit_and(str a, str b);
str bit_or(str a, str b);
str bit_xor(str a, str b);
str bit_lsh(str a, str b);
str bit_rsh(str a, str b);
bool equal(str a, str b);
bool not_equal(str a, str b);
bool less_than(str a, str b);
bool less_than_equal(str a, str b);
bool greater_than(str a, str b);
bool greater_than_equal(str a, str b);
bool logical_and(str a, str b);
bool logical_or(str a, str b);

#endif // __SEMANTICS_OPERATIONS_HH__
