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
public:
    Policy(
        std::string name,
        Position begin,
        Position end,
        std::vector<Rule *> * rules
    ) : Stmt(Node::Kind::policy, begin, end), _name(name), _rules(rules) {};

    ~Policy();

    inline std::string name() { return _name; }
    inline std::vector<Rule *> rules() { return *_rules; }

    void print(uint level = 0) override;
};

#endif // __POLICY_HH__
