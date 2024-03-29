#ifndef __WRITE_COMPILED_HH__
#define __WRITE_COMPILED_HH__

#include <fstream>
#include <linux/filter.h>
#include <compile.hh>
#include <utils/seccomp.hh>

void write_compiled(std::string filename, std::string target, CompileResult &result);

#endif // __WRITE_COMPILED_HH__
