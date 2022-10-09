#include <semantics/symbols/syscall_param.hh>

using namespace semantics;

SyscallParam::SyscallParam(std::string name, int index, bool pointer)
    : Symbol(Symbol::Kind::syscall_param, name), _index(index), _pointer(pointer) {}
