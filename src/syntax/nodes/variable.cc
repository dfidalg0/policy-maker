#include <syntax/nodes/variable.hh>

using namespace syntax;

void Variable::print(uint level) {
    std::cout << indent(level) << "> Variable: \n";
    std::cout << indent(level + 2) << "- Name: " << _name << std::endl;
}
