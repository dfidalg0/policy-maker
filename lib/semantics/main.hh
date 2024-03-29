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
#include <set>

namespace semantics {
    typedef std::vector<std::pair<std::shared_ptr<syntax::Expr>, std::shared_ptr<syntax::Action>>> SyscallRules;
    typedef std::map<unsigned, std::shared_ptr<SyscallRules>> PolicyRules;

    class Policy {
    public:
        Policy(
            std::shared_ptr<PolicyRules> rules,
            std::shared_ptr<syntax::Action> default_action
        ) : _rules(rules), _default_action(default_action) {}

        inline std::shared_ptr<PolicyRules> rules() { return _rules; }

        inline std::shared_ptr<syntax::Action> default_action() { return _default_action; }
    private:
        std::shared_ptr<syntax::Action> _default_action;
        std::shared_ptr<PolicyRules> _rules;
    };

    typedef std::unordered_map<std::string, std::shared_ptr<Policy>> Policies;

    class AnalysisResult {
    public:
        AnalysisResult(
            std::string filename,
            std::shared_ptr<Policies> policies,
            std::shared_ptr<Scope> scope
        ) : _filename(filename), _policies(policies), _scope(scope) {}

        inline std::shared_ptr<Policies> policies() { return _policies; }

        inline std::shared_ptr<Scope> scope() { return _scope; }

        inline std::string filename() { return _filename; }

        void print();
    private:
        std::shared_ptr<Policies> _policies;
        std::shared_ptr<Scope> _scope;
        std::string _filename;
    };

    std::unique_ptr<AnalysisResult> analyze(std::string filename, std::set<std::string> referrers = std::set<std::string>());
    std::unique_ptr<AnalysisResult> analyze(syntax::Program *prog, std::set<std::string> referrers = std::set<std::string>());
};

#endif // __SEMANTICS_MAIN_HH__
