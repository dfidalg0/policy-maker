#include <semantics.hh>

void compile(Program * prog) {
    // Primeiro, definimos o escopo global
    auto scope = new Scope();

    using kind = Node::Kind;

    for (auto stmt : prog->stmts()) {
        // E o atualizamos com as declarações globais
        switch (stmt->kind()) {
            case kind::function_decl: {
                auto func = new semantics::Function((FunctionDecl *) stmt, scope);
                scope->add(func);
                break;
            }
        }
    }
}
