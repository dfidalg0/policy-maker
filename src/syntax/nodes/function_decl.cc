#include <syntax/nodes/function_decl.hh>

using namespace syntax;

void FunctionDecl::print(uint level) {
    std::cout << indent(level) << "> Function Declaration:\n";
    std::cout << indent(level + 2) << "- Name: " << _name << "\n";

    std::cout << indent(level + 2) << "- Args:\n";

    for (auto arg : *_args) {
        std::cout << indent(level + 4) << "- " << arg << std::endl;
    }

    std::cout << indent(level + 2) << "- Body:\n";
    _body->print(level + 4);
}
