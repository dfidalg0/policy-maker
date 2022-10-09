#ifndef __SEMANTICS_MAIN_HH__
#define __SEMANTICS_MAIN_HH__

#include <syntax.hh>
#include <semantics.hh>
#include <utility>
#include <vector>
#include <unordered_map>
#include <string>

typedef std::vector<std::pair<Expr *, Action *>> SyscallRules;
typedef std::unordered_map<int, SyscallRules *> PolicyRules;
typedef std::unordered_map<std::string, PolicyRules *> Policies;

class AnalysisResult {
public:
    AnalysisResult(Policies *policies, Scope *scope)
        : _policies(policies), _scope(scope) {}

    inline Policies *policies() { return _policies; }

    inline Scope *scope() { return _scope; }
private:
    Policies * _policies;
    Scope * _scope;
};

AnalysisResult * analyze(std::string filename);
AnalysisResult * analyze (Program * prog);

#endif // __SEMANTICS_MAIN_HH__
