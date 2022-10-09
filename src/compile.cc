#include <compile.hh>
#include <stdexcept>

CompileResult::operator std::string() {
    throw std::runtime_error("Not implemented");
}

CompileResult::operator sock_fprog() {
    auto it = _ar->policies()->find(_target);

    if (it == _ar->policies()->end()) {
        throw std::runtime_error("Policy not found");
    }

    auto policy = it->second;
    auto filter = new std::vector<sock_filter>();

    using kind = Expr::Kind;

    filter->push_back(
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)
    );

    return sock_fprog {
        .len = (unsigned short) filter->size(),
        .filter = filter->data()
    };
}

CompileResult compile(AnalysisResult * ar, std::string target) {
    return CompileResult(ar, target);
}

CompileResult compile(Program * program, std::string target) {
    return CompileResult(program, target);
}

CompileResult compile(std::string filename, std::string target) {
    return CompileResult(filename, target);
}
