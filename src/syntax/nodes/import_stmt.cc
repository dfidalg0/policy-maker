#include <syntax/nodes/import_stmt.hh>

using namespace syntax;

void ImportStmt::print(uint level) {
    std::cout << indent(level) << "> ImportStmt\n";
    std::cout << indent(level + 2) << "- Module: " << _module << "\n";
    std::cout << indent(level + 2) << "- Imports:\n";
    for (auto import : *_imports) {
        import->print(level + 4);
    }
}
