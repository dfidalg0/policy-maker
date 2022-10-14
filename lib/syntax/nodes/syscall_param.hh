#ifndef __SYNTAX_NODES_SYSCALL_PARAM_HH__
#define __SYNTAX_NODES_SYSCALL_PARAM_HH__

#include "_expr.hh"

namespace syntax {
    class SyscallParam : public Expr {
    private:
        std::string _name;

    public:
        SyscallParam(
            std::string name,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : syntax::Expr(Node::Kind::syscall_param, begin, end), _name(name) {}

        inline std::string name() const { return _name; }

        void print(uint level = 0) override;

        uint index();
    };
};

#endif // __SYNTAX_NODES_SYSCALL_PARAM_HH__
