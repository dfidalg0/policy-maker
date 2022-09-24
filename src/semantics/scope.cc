#include <stdexcept>
#include <semantics.hh>
#include <algorithm>

using namespace semantics;

Scope Scope::add(Symbol* symbol) {
    auto name = symbol->name();

    if (_symbols.find(name) != _symbols.end()) {
        throw std::runtime_error("Symbol already defined");
    }

    _symbols[symbol->name()] = symbol;
    return *this;
}

Symbol* Scope::find(std::string name) {
    auto it = _symbols.find(name);

    if (it != _symbols.end())
        return it->second;

    if (_parent != nullptr) return _parent->find(name);

    return nullptr;
}

Expr * Scope::evaluate(Expr * expr) {
    switch (expr->kind()) {
        case Expr::Kind::function_call: {
            auto call = (FunctionCall*) expr;
            auto name = call->name();
            auto symbol = find(name);

            if (symbol == nullptr) {
                throw std::runtime_error("Function not defined");
            }

            if (symbol->kind() != Symbol::Kind::function) {
                throw std::runtime_error("Not a function");
            }

            auto function = (semantics::Function *) symbol;
            auto args = call->args();

            auto evaluated_args = std::vector<Expr*>(args.size());

            std::transform(
                args.begin(),
                args.end(),
                evaluated_args.begin(),
                [this](Expr * expr) -> Expr * {
                    return this->evaluate(expr);
                }
            );

            return function->call(evaluated_args);
        }
        case Expr::Kind::variable: {
            auto variable = (::Variable*)expr;
            auto name = variable->name();
            auto symbol = find(name);

            if (symbol == nullptr) {
                throw std::runtime_error("Variable not defined");
            }

            if (symbol->kind() != Symbol::Kind::variable) {
                throw std::runtime_error("Not a variable");
            }

            auto var = (semantics::Variable *) symbol;
            return var->value();
        }
        case Expr::Kind::unary_expr: {
            auto unary = (UnaryExpr*) expr;

            return new UnaryExpr(
                evaluate(unary->operand()),
                unary->op(),
                unary->begin(),
                unary->end()
            );
        }
        case Expr::Kind::binary_expr: {
            auto binary = (BinaryExpr*) expr;

            return new BinaryExpr(
                evaluate(binary->left()),
                evaluate(binary->right()),
                binary->op(),
                binary->begin(),
                binary->end()
            );
        }
        default: {
            return expr;
        }
    }
}
