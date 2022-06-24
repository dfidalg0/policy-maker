#include <nodes/program.h>

Program::~Program() {
    for (auto stmt : *_stmts) {
        delete stmt;
    }

    delete _stmts;
}

void Program::print(uint indent) {
    std::cout << Node::indent(indent) << "- Program: " << std::endl;

    if (_stmts->size()) {
        for (auto stmt : *_stmts) {
            stmt->print(indent + 2);
        }
    }
}
