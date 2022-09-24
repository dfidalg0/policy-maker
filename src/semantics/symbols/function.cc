#include <syntax/nodes.hh>
#include <semantics/symbols/function.hh>
#include <semantics/symbols/variable.hh>

semantics::Function::Function(
    FunctionDecl * decl,
    Scope * scope
) : Symbol(Kind::function, decl->name()) {
    _body = decl->body();
    _decl_scope = scope;
    _local_scope = new Scope(_decl_scope);

    for (auto arg: decl->args()) {
        auto var = new Variable(arg);
        _local_scope->add(var);
    }

    validate();
}

void semantics::Function::validate(Expr * expr) {
    using kind = Expr::Kind;

    if (expr == nullptr) expr = _body;

    switch (expr->kind()) {
        case kind::function_call: {
            auto call = (FunctionCall*) expr;
            auto name = call->name();
            auto symbol = _local_scope->find(name);

            if (symbol == nullptr) {
                throw std::runtime_error("Function not defined");
            }

            if (symbol->kind() != Symbol::Kind::function) {
                throw std::runtime_error("Not a function");
            }

            break;
        }
        case kind::variable: {
            auto variable = (::Variable*)expr;
            auto name = variable->name();
            auto symbol = _local_scope->find(name);

            if (symbol == nullptr) {
                throw std::runtime_error("Variable not defined");
            }

            if (symbol->kind() != Symbol::Kind::variable) {
                throw std::runtime_error("Not a variable");
            }

            break;
        }
        case kind::unary_expr: {
            auto unary = (UnaryExpr*) expr;
            validate(unary->operand());
            break;
        }
        case kind::binary_expr: {
            auto binary = (BinaryExpr*) expr;
            validate(binary->left());
            validate(binary->right());
            break;
        }
    }
};
