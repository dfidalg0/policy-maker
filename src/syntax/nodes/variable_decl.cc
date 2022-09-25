#include <syntax/nodes/variable_decl.hh>

void VariableDecl::print(uint level) {
    std::cout << indent(level) << "> Variable declaration: \n";
    std::cout << indent(level + 2) << "- Name: " << _name << std::endl;
    std::cout << indent(level + 2) << "- Value:\n";
    _value->print(level + 4);
}
