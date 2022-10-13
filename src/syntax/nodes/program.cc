#include <syntax/nodes/program.hh>

void Program::print(uint level) {
    std::cout << indent(level) << "> Program: " << std::endl;

    if (_stmts->empty()) {
        std::cout << indent(level + 2) << "No statements" << std::endl;

        return;
    }

    for (auto stmt : *_stmts) {
        stmt->print(level + 2);
    }
}
