#ifndef __RULE_HH__
#define __RULE_HH__

#include "_node.hh"
#include "action.hh"
#include "syscall.hh"
#include <vector>

class Rule : public Node {
private:
    Action * _action;
    std::vector<Syscall *> * _syscalls;

public:
    Rule(Action * action, std::vector<Syscall *> * syscalls, Position begin, Position end)
        : Node(Node::Kind::rule, begin, end), _action(action), _syscalls(syscalls) {}
    ~Rule();

    inline Action * action() { return _action; }
    inline std::vector<Syscall *> syscalls() { return *_syscalls; }

    void print(uint level = 0) override;
};

#endif // __RULE_HH__
