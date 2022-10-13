#ifndef __COMPILE_HH__
#define __COMPILE_HH__

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <syscall.h>

#include <semantics/main.hh>
#include <compile_expr.hh>

#include <string>
#include <vector>

class CompileResult {
public:
    CompileResult(AnalysisResult * ar, std::string entry);
    CompileResult(Program * program, std::string entry);
    CompileResult(std::string filename, std::string entry);

    operator std::string();

    operator sock_fprog();
private:
    std::unique_ptr<FilterVector> _filter;
};

CompileResult compile(AnalysisResult * ar, std::string entry);
CompileResult compile(Program * program, std::string entry);
CompileResult compile(std::string filename, std::string entry);

#endif // __COMPILE_HH__
