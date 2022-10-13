#include <syntax/nodes/function_call.hh>

void FunctionCall::print(uint level) {
    std::cout << indent(level) << "> Function Call:\n";
    std::cout << indent(level + 2) << "- Name: " << _name << "\n";

    if (_args->empty()) {
        return;
    }

    std::cout << indent(level + 2) << "- Parameters:" << std::endl;

    for (auto arg : *_args) {
        arg->print(level + 4);
    }
}
