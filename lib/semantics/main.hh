#ifndef __SEMANTICS_MAIN_HH__
#define __SEMANTICS_MAIN_HH__

#include <syntax.hh>
#include <semantics.hh>
#include <utility>
#include <vector>
#include <unordered_map>
#include <string>
#include <map>
#include <memory>

typedef std::vector<std::pair<std::shared_ptr<Expr>, std::shared_ptr<Action>>> SyscallRules;
typedef std::map<unsigned, SyscallRules *> PolicyRules;

class AnalysisResultPolicy {
public:
    AnalysisResultPolicy(PolicyRules * rules, Action * default_action)
        : _rules(rules), _default_action(default_action) {}

    inline PolicyRules * rules() { return _rules; }

    inline Action * default_action() { return _default_action; }
private:
    Action * _default_action;
    PolicyRules * _rules;
};

typedef std::unordered_map<std::string, AnalysisResultPolicy *> Policies;

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

std::unique_ptr<AnalysisResult> analyze(std::string filename);
std::unique_ptr<AnalysisResult> analyze(Program *prog);

#endif // __SEMANTICS_MAIN_HH__
