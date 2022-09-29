#include <stdexcept>
#include <syntax.hh>
#include <semantics.hh>
#include <algorithm>
#include <semantics/operations.h>

using namespace semantics;

template <class L, class R>
static Constant *operate(Constant *a, BinaryExpr::OpKind op, Constant *b) {
    L left = *a;
    R right = *b;

    using opKind = BinaryExpr::OpKind;

    switch (op) {
        case opKind::add:
            return new Constant(
                add(left, right),
                a->begin(),
                b->end());
        case opKind::sub:
            return new Constant(
                subtract(left, right),
                a->begin(),
                b->end());
        case opKind::mul:
            return new Constant(
                multiply(left, right),
                a->begin(),
                b->end());
        case opKind::div:
            return new Constant(
                divide(left, right),
                a->begin(),
                b->end());
        case opKind::mod:
            return new Constant(
                modulo(left, right),
                a->begin(),
                b->end());
        case opKind::bit_and:
            return new Constant(
                bit_and(left, right),
                a->begin(),
                b->end());
        case opKind::bit_or:
            return new Constant(
                bit_or(left, right),
                a->begin(),
                b->end());
        case opKind::bit_xor:
            return new Constant(
                bit_xor(left, right),
                a->begin(),
                b->end());
        case opKind::bit_lsh:
            return new Constant(
                bit_lsh(left, right),
                a->begin(),
                b->end());
        case opKind::bit_rsh:
            return new Constant(
                bit_rsh(left, right),
                a->begin(),
                b->end());
        case opKind::eq:
            return new Constant(
                equal(left, right),
                a->begin(),
                b->end());
        case opKind::ne:
            return new Constant(
                not_equal(left, right),
                a->begin(),
                b->end());
        case opKind::lt:
            return new Constant(
                less_than(left, right),
                a->begin(),
                b->end());
        case opKind::le:
            return new Constant(
                less_than_equal(left, right),
                a->begin(),
                b->end());
        case opKind::gt:
            return new Constant(
                greater_than(left, right),
                a->begin(),
                b->end());
        case opKind::ge:
            return new Constant(
                greater_than_equal(left, right),
                a->begin(),
                b->end());
        case opKind::land:
            return new Constant(
                logical_and(left, right),
                a->begin(),
                b->end());
        case opKind::lor:
            return new Constant(
                logical_or(left, right),
                a->begin(),
                b->end());
        default:
            throw std::runtime_error("Invalid operation");
    }
}

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
    if (!expr) return nullptr;

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
            int value = *constant;

            switch (op) {
                case opKind::neg: {
                    return new Constant(-value, begin, end);
                }
                case opKind::pos: {
                    return new Constant(+value, begin, end);
                }
                case opKind::bit_not: {
                    return new Constant(~value, begin, end);
                }
                case opKind::lnot: {
                    return new Constant(!value, begin, end);
                }
            }
        }
        case type::boolean: {
            bool value = *constant;

            switch (op) {
                case opKind::neg: {
                    return new Constant(-value, begin, end);
                }
                case opKind::pos: {
                    return new Constant(+value, begin, end);
                }
                case opKind::bit_not: {
                    return new Constant(~value, begin, end);
                }
                case opKind::lnot: {
                    return new Constant(!value, begin, end);
                }
            }
        }
        case type::null: {
            switch(op) {
                case opKind::lnot: {
                    return new Constant(true, begin, end);
                }
                default: {
                    throw std::runtime_error("Invalid operation");
                }
            }
        }
        default: {
            std::string value = *constant;

            switch (op) {
                case opKind::lnot: {
                    return new Constant(!value.empty(), begin, end);
                }
                default: {
                    throw std::runtime_error("Invalid operation");
                }
            }
        }
    }
}

Expr * Scope::simplify(BinaryExpr * binary) {
    using type = Constant::Type;
    using kind = Expr::Kind;

    auto left = evaluate(binary->left());
    auto right = evaluate(binary->right());
    auto op = binary->op();
    auto begin = binary->begin();
    auto end = binary->end();

    // TODO: implementar simplificação com base na associatividade dos
    // operadores

    if (
        left->kind() != kind::constant ||
        right->kind() != kind::constant
    ) {
        return new BinaryExpr(
            left,
            right,
            op,
            begin,
            end
        );
    }

    auto cl = (Constant *) left;
    auto cr = (Constant *) right;

    if (cl->type() == type::null || cr->type() == type::null) {
        throw std::runtime_error("Invalid operation");
    }

    switch (cl->type()) {
        case type::integer:
            switch (cr->type()) {
                case type::integer:
                case type::boolean:
                    return operate<int, int>(cl, op, cr);
                default:
                    return operate<std::string, std::string>(cl, op, cr);
            }
        case type::boolean:
            switch (cr->type()) {
                case type::integer:
                case type::boolean:
                    return operate<bool, bool>(cl, op, cr);
                default:
                    return operate<std::string, std::string>(cl, op, cr);
            }
        // type::string
        default:
            return operate<std::string, std::string>(cl, op, cr);
    }

    return new BinaryExpr(left, right, op, begin, end);
}
