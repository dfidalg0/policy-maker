#include <syntax/nodes/rule.hh>

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
