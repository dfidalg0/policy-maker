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

            return simplify(unary);
        }
        case Expr::Kind::binary_expr: {
            auto binary = (BinaryExpr*) expr;

            return simplify(binary);
        }
        default: {
            return expr;
        }
    }
}

Expr * Scope::simplify(UnaryExpr * unary) {
    using type = Constant::Type;
    using opKind = UnaryExpr::OpKind;

    auto operand = evaluate(unary->operand());
    auto op = unary->op();
    auto begin = unary->begin();
    auto end = unary->end();

    if (operand->kind() != Expr::Kind::constant) {
        return new UnaryExpr(
            operand,
            op,
            begin,
            end
        );
    }

    auto constant = (Constant *) operand;

    switch (constant->type()) {
        case type::integer: {
            auto value = std::stoi(constant->value());

            switch (op) {
                case opKind::neg: {
                    return new Constant(
                        type::integer, std::to_string(-value), begin, end
                    );
                }
                case opKind::pos: {
                    return new Constant(
                        type::integer, std::to_string(+value), begin, end
                    );
                }
                case opKind::bit_not: {
                    return new Constant(
                        type::integer, std::to_string(~value), begin, end
                    );
                }
                case opKind::lnot: {
                    return new Constant(
                        type::boolean, std::to_string(!value), begin, end
                    );
                }
            }
        }
        case type::boolean: {
            auto raw = constant->value();
            auto value = raw == "1" || raw == "true";

            switch (op) {
                case opKind::neg: {
                    return new Constant(
                        type::integer, std::to_string(-value), begin, end
                    );
                }
                case opKind::pos: {
                    return new Constant(
                        type::integer, std::to_string(+value), begin, end
                    );
                }
                case opKind::bit_not: {
                    return new Constant(
                        type::integer, std::to_string(~value), begin, end
                    );
                }
                case opKind::lnot: {
                    return new Constant(
                        type::boolean, std::to_string(!value), begin, end
                    );
                }
            }
        }
        case type::null: {
            switch(op) {
                case opKind::lnot: {
                    return new Constant(
                        type::boolean, "true", begin, end
                    );
                }
                default: {
                    throw std::runtime_error("Invalid operation");
                }
            }
        }
        default: {
            auto value = constant->value();

            switch (op) {
                case opKind::lnot: {
                    return new Constant(
                        type::boolean, std::to_string(!value.empty()), begin, end
                    );
                }
                default: {
                    throw std::runtime_error("Invalid operation");
                }
            }
        }
    }
}

Expr * Scope::simplify(BinaryExpr * binary) {
    auto left = evaluate(binary->left());
    auto right = evaluate(binary->right());
    auto op = binary->op();
    auto begin = binary->begin();
    auto end = binary->end();

    return new BinaryExpr(left, right, op, begin, end);
}
