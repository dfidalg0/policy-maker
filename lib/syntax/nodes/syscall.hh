#ifndef __SYNTAX_NODES_SYSCALL_HH__
#define __SYNTAX_NODES_SYSCALL_HH__

#include "_node.hh"
#include "_expr.hh"
#include <string>
#include <memory>

namespace syntax {
    class Syscall: public Node {
    public:
        Syscall(
            std::string name,
            std::shared_ptr<Expr> condition = nullptr,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Node(Node::Kind::syscall, begin, end), _name(name), _condition(condition) {};

        void print(uint level = 0) override;

        inline std::string name() const { return _name; }
        inline std::shared_ptr<Expr> condition() const { return _condition; }

    private:
        std::string _name;
        std::shared_ptr<Expr> _condition;
    };
}

#endif // __SYNTAX_NODES_SYSCALL_HH__
