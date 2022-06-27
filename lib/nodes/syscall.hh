#ifndef __SYSCALL_HH__
#define __SYSCALL_HH__

#include <nodes/_node.hh>
#include <nodes/_expr.hh>
#include <string>

class Syscall: public Node {
public:
    Syscall(
        std::string name,
        Position begin,
        Position end,
        Expr * condition = nullptr
    ) : Node(Node::Kind::syscall, begin, end), _name(name), _condition(condition) {};

    ~Syscall();

    void print(uint indent = 0) override;

private:
    std::string _name;
    Expr * _condition;
};

#endif // __SYSCALL_HH__
