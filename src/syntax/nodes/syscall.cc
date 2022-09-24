#include <syntax/nodes/syscall.hh>

Syscall::~Syscall() {
    if (!_condition) return;

    delete _condition;
}

void Syscall::print(uint level) {
    std::cout << indent(level) << "> Syscall: " << _name << std::endl;

    if (!_condition) return;

    std::cout << indent(level + 2) << "- Condition: " << std::endl;
    _condition->print(level + 4);
}
