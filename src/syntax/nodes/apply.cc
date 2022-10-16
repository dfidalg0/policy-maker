#include <syntax/nodes/apply.hh>

using namespace syntax;

void Apply::print(uint level) {
    std::cout << indent(level) << "> Apply: " << _policy << '\n';
}
