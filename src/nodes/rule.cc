#include <nodes/rule.hh>

Rule::~Rule() {
    delete _action;
    for (auto * syscall : *_syscalls) {
        delete syscall;
    }
    delete _syscalls;
}

void Rule::print(uint indent) {
    std::cout << Node::indent(indent) << "- Rule: " << std::endl;

    _action->print(indent + 2);

    if (_syscalls->size()) {
        std::cout << Node::indent(indent + 2) << "- Syscalls: " << std::endl;
        for (auto * syscall : *_syscalls) {
            syscall->print(indent + 4);
        }
    }
}
