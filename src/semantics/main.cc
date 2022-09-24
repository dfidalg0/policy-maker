#include <semantics.hh>

void compile(Program * prog) {
    auto scope = new Scope();

    using kind = Node::Kind;

    for (auto stmt : prog->stmts()) {
        switch (stmt->kind()) {
            case kind::function_decl: {
                auto func = new semantics::Function((FunctionDecl *) stmt, scope);
                scope->add(func);
                break;
            }
        }
    }
}
