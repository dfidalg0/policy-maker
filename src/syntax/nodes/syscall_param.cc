#include <syntax/nodes/syscall_param.hh>

void SyscallParam::print(uint level) {
    std::cout << indent(level) << "> SyscallParam: \n";

    try {
        std::cout << indent(level + 2) << "- Index: " << std::stoi(_name) << "\n";
    }
    catch (std::invalid_argument) {
        std::cout << indent(level + 2) << "- Name: " << _name << std::endl;
    }
}
