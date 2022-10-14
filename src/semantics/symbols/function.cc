#include <syntax/nodes.hh>
#include <semantics/symbols/function.hh>
#include <semantics/symbols/variable.hh>

using namespace semantics;

using syntax::FunctionDecl;
using syntax::Expr;
using syntax::UnaryExpr;
using syntax::BinaryExpr;

Function::Function(
    FunctionDecl * decl,
    std::shared_ptr<Scope> scope
) : Symbol(Kind::function, decl->name()) {
    _body = decl->body();
    _decl_scope = scope;

    _args = decl->args();

    validate();
}

static void validate_expr(Scope * scope, syntax::Expr * expr) {
    using kind = syntax::Expr::Kind;

    switch (expr->kind()) {
        case kind::function_call: {
            auto call = (syntax::FunctionCall*)expr;
            auto name = call->name();
            auto symbol = scope->find(name);

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
            auto symbol = scope->find(name);

            if (symbol == nullptr) {
                throw std::runtime_error("Variable not defined");
            }

            if (symbol->kind() != Symbol::Kind::variable) {
                throw std::runtime_error("Not a variable");
            }

            break;
        }
        case kind::unary_expr: {
            auto unary = (UnaryExpr*)expr;
            validate_expr(scope, unary->operand().get());
            break;
        }
        case kind::binary_expr: {
            auto binary = (BinaryExpr*)expr;
            validate_expr(scope, binary->left().get());
            validate_expr(scope, binary->right().get());
            break;
        }
        case kind::syscall_param: {
            throw std::runtime_error(
                "Syscall param not allowed inside function body"
            );
        }
    }
}

void Function::validate(syntax::Expr * expr) {
    if (expr == nullptr) expr = _body.get();

    auto local_scope = new Scope(_decl_scope);

    for (auto arg : _args) {
        auto var = std::make_shared<Variable>(arg);
        local_scope->add(var);
    }

    try {
        validate_expr(local_scope, expr);
    }
    catch (std::runtime_error & e) {
        delete local_scope;
        throw e;
    }

    delete local_scope;
};

std::shared_ptr<syntax::Expr> Function::call(std::vector<std::shared_ptr<syntax::Expr>> args) {
    auto local_scope = new Scope(_decl_scope);

    if (args.size() < _args.size()) {
        throw std::runtime_error("Invalid number of arguments");
    }

    for (int i = 0; i < _args.size(); i++) {
        auto arg = _args[i];
        auto value = args[i];

        auto var = std::make_shared<Variable>(arg, value);
        local_scope->add(var);
    }

    auto result = local_scope->evaluate(_body);

    delete local_scope;

    return result;
}
