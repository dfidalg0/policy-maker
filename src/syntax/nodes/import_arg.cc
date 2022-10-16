#include <syntax/nodes/import_arg.hh>

using namespace syntax;

void ImportArg::print(uint level) {
    std::cout << indent(level) << "> ImportArg\n";
    std::cout << indent(level + 2) << "- Name: ";

    if (_policy) {
        std::cout << "policy ";
    }

    std::cout << _name << '\n';

    if (_aliased) {
        std::cout << indent(level + 2) << "- Alias: " << _alias << "\n";
    }
}
