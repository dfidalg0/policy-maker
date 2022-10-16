#include <syntax/nodes.hh>
#include <semantics/symbols/function.hh>
#include <semantics/symbols/variable.hh>
#include <errors.hh>

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

    try {
        validate();
    }
    catch (CompilerError& e) {
        throw e
            .push(_body->begin(), "function body");
    }
}

static void validate_expr(Scope * scope, syntax::Expr * expr) {
    using kind = syntax::Expr::Kind;

    switch (expr->kind()) {
        case kind::function_call: {
            auto call = (syntax::FunctionCall*)expr;
            auto name = call->name();
            auto symbol = scope->find(name);

            if (symbol == nullptr) {
                throw CompilerError("Function not defined: " + name)
                    .push(call->begin(), "function call");
            }

            if (symbol->kind() != Symbol::Kind::function) {
                throw CompilerError("Not a function: " + name)
                    .push(call->begin(), "function call");
            }

            break;
        }
        case kind::variable: {
            auto variable = (syntax::Variable*)expr;
            auto name = variable->name();
            auto symbol = scope->find(name);

            if (symbol == nullptr) {
                throw CompilerError("Variable not defined: " + name)
                    .push(variable->begin(), "variable access");
            }

            if (symbol->kind() != Symbol::Kind::variable) {
                throw CompilerError("Not a variable: " + name)
                    .push(variable->begin(), "variable access");
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
            auto param = (syntax::SyscallParam*)expr;

            throw CompilerError("Syscall param not allowed inside function body")
                .push(param->begin(), "syscall param access");
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
    catch (CompilerError & e) {
        delete local_scope;
        throw e;
    }

    delete local_scope;
};

std::shared_ptr<syntax::Expr> Function::call(std::vector<std::shared_ptr<syntax::Expr>> args) {
    auto local_scope = new Scope(_decl_scope);

    if (args.size() < _args.size()) {
        throw CompilerError(
            "Invalid number of arguments. Expected" + std::to_string(args.size()) + "\n"
            "  at function " + name()
        );
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
