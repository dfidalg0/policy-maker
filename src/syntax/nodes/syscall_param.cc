#include <syntax/nodes/syscall_param.hh>

void SyscallParam::print(uint level) {
    std::cout << indent(level) << "> SyscallParam: \n";
    std::cout << indent(level + 2) << "- Name: " << _name << std::endl;
}
