#ifndef __COMPILE_HH__
#define __COMPILE_HH__

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <syscall.h>

#include <semantics/main.hh>
#include <string>
#include <vector>

class CompileResult {
public:
    CompileResult(AnalysisResult * ar, std::string target);
    CompileResult(Program * program, std::string target);
    CompileResult(std::string filename, std::string target);

    operator std::string();

    operator sock_fprog();
private:
    std::vector<sock_filter> * _filter;
};

CompileResult compile(AnalysisResult * ar, std::string target);
CompileResult compile(Program * program, std::string target);
CompileResult compile(std::string filename, std::string target);

#endif // __COMPILE_HH__
