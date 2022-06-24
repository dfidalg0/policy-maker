#ifndef __SYSCALL_HH__
#define __SYSCALL_HH__

#include <nodes/_node.hh>
#include <string>

class Syscall: public Node {
public:
    Syscall(
        std::string name,
        Position begin,
        Position end,
        Node * condition = nullptr
    ) : Node(Node::Kind::syscall, begin, end), _name(name), _condition(condition) {};

    ~Syscall();

    void print(uint indent = 0) override;

private:
    std::string _name;
    Node * _condition;
};

#endif // __SYSCALL_HH__
