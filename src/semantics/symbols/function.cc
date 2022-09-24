#include <syntax/nodes.hh>
#include <semantics/symbols/function.hh>
#include <semantics/symbols/variable.hh>

using namespace semantics;

Function::Function(
    FunctionDecl * decl,
    Scope * scope
) : Symbol(Kind::function, decl->name()) {
    _body = decl->body();
    _decl_scope = scope;

    _args = decl->args();

    validate();
}

void Function::validate(Expr * expr) {
    using kind = Expr::Kind;

    if (expr == nullptr) expr = _body;

    auto local_scope = new Scope(_decl_scope);

    for (auto arg : _args) {
        auto var = new Variable(arg);
        local_scope->add(var);
    }

    switch (expr->kind()) {
        case kind::function_call: {
            auto call = (FunctionCall*) expr;
            auto name = call->name();
            auto symbol = local_scope->find(name);

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
            auto symbol = local_scope->find(name);

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

    delete local_scope;
};

Expr * Function::call(std::vector<Expr*> args) {
    auto local_scope = new Scope(_decl_scope);

    if (args.size() < _args.size()) {
        throw std::runtime_error("Invalid number of arguments");
    }

    for (int i = 0; i < _args.size(); i++) {
        auto arg = _args[i];
        auto value = args[i];

        auto var = new Variable(arg, value);
        local_scope->add(var);
    }

    auto result = local_scope->evaluate(_body);

    delete local_scope;

    return result;
}
