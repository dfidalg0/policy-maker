#include <syntax/nodes/syscall_param.hh>

using namespace syntax;

void SyscallParam::print(uint level) {
    std::cout << indent(level) << "> SyscallParam: \n";

    try {
        auto idx = std::stoi(_name);
        std::cout << indent(level + 2) << "- Index: " << idx << "\n";
    }
    catch (std::invalid_argument) {
        std::cout << indent(level + 2) << "- Name: " << _name << std::endl;
    }
}

uint SyscallParam::index() {
    return std::stoi(_name);
}
