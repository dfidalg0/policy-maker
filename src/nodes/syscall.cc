#include <nodes/syscall.hh>

Syscall::~Syscall() {
    if (!_condition) return;

    delete _condition;
}

void Syscall::print(uint indent) {
    std::cout << Node::indent(indent) << "- Syscall: " << _name << std::endl;

    if (!_condition) return;

    std::cout << Node::indent(indent + 2) << "- Condition: " << std::endl;
    _condition->print(indent + 4);
}
