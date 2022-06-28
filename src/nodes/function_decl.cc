#include <nodes/function_decl.hh>

FunctionDecl::~FunctionDecl() {
    delete _body;
    delete _args;
}

void FunctionDecl::print(uint level) {
    std::cout << indent(level) << "- FunctionDecl:\n";
    std::cout << indent(level + 2) << "- Name: " << _name << "\n";

    std::cout << indent(level + 2) << "- Args:\n";

    for (auto arg : *_args) {
        std::cout << indent(level + 4) << "- " << arg << std::endl;
    }

    std::cout << indent(level + 2) << "- Body:\n";
    _body->print(level + 4);
}
