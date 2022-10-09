#ifndef __POLICY_HH__
#define __POLICY_HH__

#include "_stmt.hh"
#include "rule.hh"
#include <string>
#include <vector>

class Policy: public Stmt {
private:
    std::string _name;
    std::vector<Rule *> * _rules;
    Action * _default_action;
public:
    Policy(
        std::string name,
        std::vector<Rule *> * rules,
        Action * default_action,
        Position begin,
        Position end
    ) : Stmt(Node::Kind::policy, begin, end),
        _name(name), _rules(rules), _default_action(default_action) {}

    ~Policy();

    inline std::string name() { return _name; }
    inline std::vector<Rule *> rules() { return *_rules; }
    inline Action * default_action() { return _default_action; }

    void print(uint level = 0) override;
};

#endif // __POLICY_HH__
