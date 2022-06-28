#include <nodes/rule.hh>

Rule::~Rule() {
    delete _action;
    for (auto * syscall : *_syscalls) {
        delete syscall;
    }
    delete _syscalls;
}

void Rule::print(uint level) {
    std::cout << indent(level) << "> Rule: " << std::endl;

    _action->print(level + 2);

    if (_syscalls->size()) {
        std::cout << indent(level + 2) << "- Syscalls: " << std::endl;
        for (auto * syscall : *_syscalls) {
            syscall->print(level + 4);
        }
    }
}
