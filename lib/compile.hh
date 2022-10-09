#ifndef __COMPILE_HH__
#define __COMPILE_HH__

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <syscall.h>

#include <semantics/main.hh>
#include <string>

class CompileResult {
public:
    CompileResult(AnalysisResult * ar, std::string target)
        : _ar(ar), _target(target) {}
    CompileResult(Program * program, std::string target)
        : CompileResult(analyze(program), target) {}
    CompileResult(std::string filename, std::string target)
        : CompileResult(analyze(filename), target) {}

    operator std::string();

    operator sock_fprog();
private:
    std::string _target;
    AnalysisResult * _ar;
};

CompileResult compile(AnalysisResult * ar, std::string target);
CompileResult compile(Program * program, std::string target);
CompileResult compile(std::string filename, std::string target);

#endif // __COMPILE_HH__
