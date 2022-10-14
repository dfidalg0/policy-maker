#ifndef __SYNTAX_NODES_RULE_HH__
#define __SYNTAX_NODES_RULE_HH__

#include "_node.hh"
#include "action.hh"
#include "syscall.hh"
#include <vector>
#include <memory>

namespace syntax {
    typedef std::vector<std::shared_ptr<Syscall>> SyscallsList;

    class Rule : public Node {
    private:
        std::shared_ptr<Action> _action;
        std::shared_ptr<SyscallsList> _syscalls;

    public:
        Rule(
            std::shared_ptr<Action> action,
            std::shared_ptr<SyscallsList> syscalls,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Node(Node::Kind::rule, begin, end), _action(action), _syscalls(syscalls) {}

        inline std::shared_ptr<Action> action() const { return _action; }
        inline SyscallsList syscalls() { return *_syscalls; }

        void print(uint level = 0) override;
    };
}


#endif // __SYNTAX_NODES_RULE_HH__
