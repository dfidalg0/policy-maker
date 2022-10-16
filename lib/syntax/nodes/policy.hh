#ifndef __POLICY_HH__
#define __POLICY_HH__

#include "_stmt.hh"
#include "rule.hh"
#include <string>
#include <vector>
#include <memory>

namespace syntax {
    typedef std::vector<std::shared_ptr<Rule>> RulesList;
    typedef std::vector<std::shared_ptr<Stmt>> RuleBody;

    class Policy: public Stmt {
    private:
        std::string _name;
        std::shared_ptr<RuleBody> _body;
        std::shared_ptr<Action> _default_action;
    public:
        Policy(
            std::string name,
            std::shared_ptr<RuleBody> body,
            std::shared_ptr<Action> default_action,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Stmt(Node::Kind::policy, begin, end),
            _name(name), _body(body), _default_action(default_action) {}

        inline std::string name() { return _name; }
        inline RuleBody body() { return *_body; }
        inline std::shared_ptr<Action> default_action() { return _default_action; }

        void print(uint level = 0) override;
    };
};

#endif // __POLICY_HH__
