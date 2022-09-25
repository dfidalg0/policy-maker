#ifndef __SYSCALL_PARAM_HH__
#define __SYSCALL_PARAM_HH__

#include "_expr.hh"

class SyscallParam : public Expr {
private:
    std::string _name;

public:
    SyscallParam(std::string name, Position begin, Position end)
        : Expr(Node::Kind::syscall_param, begin, end), _name(name) {}

    ~SyscallParam() {}

    inline std::string name() const { return _name; }

    void print(uint level = 0) override;
};

#endif // __SYSCALL_PARAM_HH__
