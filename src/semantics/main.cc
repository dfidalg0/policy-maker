#include <semantics.hh>
#include <syntax/nodes/function_decl.hh>
#include <syntax/nodes/variable_decl.hh>

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
            case kind::variable_decl: {
                auto var_decl = (VariableDecl *) stmt;

                auto name = var_decl->name();
                auto value = var_decl->value();

                auto var = new semantics::Variable(name, scope->evaluate(value));
                scope->add(var);
                break;
            }
        }
    }
}
