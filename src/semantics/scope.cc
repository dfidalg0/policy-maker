#include <stdexcept>
#include <syntax.hh>
#include <semantics.hh>
#include <algorithm>
#include <memory>
#include <errors.hh>
#include <semantics/operations.hh>

using namespace semantics;
using syntax::Constant;
using syntax::BinaryExpr;
using syntax::UnaryExpr;

template <class L, class R>
static std::unique_ptr<Constant> operate(Constant *a, BinaryExpr::OpKind op, Constant *b) {
    L left = *a;
    R right = *b;

    using opKind = BinaryExpr::OpKind;

    switch (op) {
        case opKind::add:
            return std::make_unique<Constant>(
                add(left, right),
                a->begin(),
                b->end()
            );
        case opKind::sub:
            return std::make_unique<Constant>(
                subtract(left, right),
                a->begin(),
                b->end()
            );
        case opKind::mul:
            return std::make_unique<Constant>(
                multiply(left, right),
                a->begin(),
                b->end()
            );
        case opKind::div:
            return std::make_unique<Constant>(
                divide(left, right),
                a->begin(),
                b->end()
            );
        case opKind::mod:
            return std::make_unique<Constant>(
                modulo(left, right),
                a->begin(),
                b->end()
            );
        case opKind::bit_and:
            return std::make_unique<Constant>(
                bit_and(left, right),
                a->begin(),
                b->end()
            );
        case opKind::bit_or:
            return std::make_unique<Constant>(
                bit_or(left, right),
                a->begin(),
                b->end()
            );
        case opKind::bit_xor:
            return std::make_unique<Constant>(
                bit_xor(left, right),
                a->begin(),
                b->end()
            );
        case opKind::bit_lsh:
            return std::make_unique<Constant>(
                bit_lsh(left, right),
                a->begin(),
                b->end()
            );
        case opKind::bit_rsh:
            return std::make_unique<Constant>(
                bit_rsh(left, right),
                a->begin(),
                b->end()
            );
        case opKind::eq:
            return std::make_unique<Constant>(
                equal(left, right),
                a->begin(),
                b->end()
            );
        case opKind::ne:
            return std::make_unique<Constant>(
                not_equal(left, right),
                a->begin(),
                b->end()
            );
        case opKind::lt:
            return std::make_unique<Constant>(
                less_than(left, right),
                a->begin(),
                b->end()
            );
        case opKind::le:
            return std::make_unique<Constant>(
                less_than_equal(left, right),
                a->begin(),
                b->end()
            );
        case opKind::gt:
            return std::make_unique<Constant>(
                greater_than(left, right),
                a->begin(),
                b->end()
            );
        case opKind::ge:
            return std::make_unique<Constant>(
                greater_than_equal(left, right),
                a->begin(),
                b->end()
            );
        case opKind::land:
            return std::make_unique<Constant>(
                logical_and(left, right),
                a->begin(),
                b->end()
            );
        case opKind::lor:
            return std::make_unique<Constant>(
                logical_or(left, right),
                a->begin(),
                b->end()
            );
        default:
            throw CompilerError("Invalid operation");
    }
}

Scope& Scope::add(std::shared_ptr<Symbol> symbol) {
    auto name = symbol->name();

    add(name, symbol);

    return *this;
}

Scope& Scope::add(const std::string& name, std::shared_ptr<Symbol> symbol) {
    if (_symbols.find(name) != _symbols.end()) {
        throw CompilerError("Symbol already defined: " + name);
    }

    _symbols[name] = symbol;
    return *this;
}

std::shared_ptr<Symbol> Scope::find(std::string name) {
    auto it = _symbols.find(name);

    if (it != _symbols.end())
        return it->second;

    if (_parent != nullptr) return _parent->find(name);

    return nullptr;
}

std::shared_ptr<syntax::Expr> Scope::evaluate(std::shared_ptr<syntax::Expr> expr) {
    if (!expr) return nullptr;

    switch (expr->kind()) {
        case syntax::Expr::Kind::function_call: {
            auto call = std::static_pointer_cast<syntax::FunctionCall>(expr);
            auto name = call->name();
            auto symbol = find(name);

            if (symbol == nullptr) {
                throw CompilerError("Function not defined: " + name)
                    .push(call->begin(), "function call");
            }

            if (symbol->kind() != Symbol::Kind::function) {
                throw CompilerError("Not a function: " + name)
                    .push(call->begin(), "function call");
            }

            auto function = std::static_pointer_cast<semantics::Function>(symbol);
            auto args = call->args();

            auto evaluated_args = std::vector<std::shared_ptr<syntax::Expr>>(args.size());

            try {
                std::transform(
                    args.begin(),
                    args.end(),
                    evaluated_args.begin(),
                    [this](std::shared_ptr<syntax::Expr> expr) {
                        return this->evaluate(expr);
                    }
                );
            }
            catch (CompilerError& e) {
                throw e.push(call->begin(), "function call");
            }

            try {
                return function->call(evaluated_args);
            }
            catch (CompilerError &e) {
                throw e.push(call->begin(), "function call");
            }
        }
        case syntax::Expr::Kind::variable: {
            auto variable = std::static_pointer_cast<syntax::Variable>(expr);
            auto name = variable->name();
            auto symbol = find(name);

            if (symbol == nullptr) {
                throw CompilerError("Variable not defined: " + name)
                    .push(variable->begin(), "variable access");
            }

            if (symbol->kind() != Symbol::Kind::variable) {
                throw CompilerError("Not a variable: " + name)
                    .push(variable->begin(), "variable access");
            }

            auto var = std::static_pointer_cast<semantics::Variable>(symbol);
            return var->value();
        }
        case syntax::Expr::Kind::syscall_param: {
            auto param = std::static_pointer_cast<syntax::SyscallParam>(expr);

            try {
                std::stoi(param->name());
                return param;
            } catch (std::invalid_argument) {
                auto name = param->name();

                auto symbol = find(":" + name);

                if (symbol == nullptr || symbol->kind() != Symbol::Kind::syscall_param) {
                    throw CompilerError("Syscall parameter not defined: " + name)
                        .push(param->begin(), "syscall parameter access");
                }

                auto sys_param = std::static_pointer_cast<semantics::SyscallParam>(symbol);

                auto index = sys_param->index();

                return std::make_shared<syntax::SyscallParam>(
                    std::to_string(index),
                    param->begin(),
                    param->end()
                );
            }
        }
        case syntax::Expr::Kind::unary_expr: {
            auto unary = std::static_pointer_cast<UnaryExpr>(expr);

            try {
                return simplify(unary);
            }
            catch (CompilerError & e) {
                throw e
                    .push(unary->begin(), "expression");
            }
        }
        case syntax::Expr::Kind::binary_expr: {
            auto binary = std::static_pointer_cast<BinaryExpr>(expr);

            try {
                return simplify(binary);
            }
            catch (CompilerError & e) {
                throw e
                    .push(binary->begin(), "expression");
            }
        }
        default: {
            return expr;
        }
    }
}

std::shared_ptr<syntax::Expr> Scope::simplify(std::shared_ptr<UnaryExpr> unary) {
    using type = Constant::Type;
    using opKind = UnaryExpr::OpKind;

    auto operand = evaluate(unary->operand());
    auto op = unary->op();
    auto begin = unary->begin();
    auto end = unary->end();

    if (operand->kind() != syntax::Expr::Kind::constant) {
        return std::make_unique<UnaryExpr>(
            operand,
            op,
            begin,
            end
        );
    }

    auto constant = std::static_pointer_cast<Constant>(operand);

    switch (constant->type()) {
        case type::integer: {
            int value = *constant;

            switch (op) {
                case opKind::neg: {
                    return std::make_unique<Constant>(-value, begin, end);
                }
                case opKind::pos: {
                    return std::make_unique<Constant>(+value, begin, end);
                }
                case opKind::bit_not: {
                    return std::make_unique<Constant>(~value, begin, end);
                }
                case opKind::lnot: {
                    return std::make_unique<Constant>(!value, begin, end);
                }
            }
        }
        case type::boolean: {
            bool value = *constant;

            switch (op) {
                case opKind::neg: {
                    return std::make_unique<Constant>(-value, begin, end);
                }
                case opKind::pos: {
                    return std::make_unique<Constant>(+value, begin, end);
                }
                case opKind::bit_not: {
                    return std::make_unique<Constant>(~value, begin, end);
                }
                case opKind::lnot: {
                    return std::make_unique<Constant>(!value, begin, end);
                }
            }
        }
        case type::null: {
            switch(op) {
                case opKind::lnot: {
                    return std::make_unique<Constant>(true, begin, end);
                }
                default: {
                    throw CompilerError("Invalid operation with null");
                }
            }
        }
        default: {
            std::string value = *constant;

            switch (op) {
                case opKind::lnot: {
                    return std::make_unique<Constant>(!value.empty(), begin, end);
                }
                default: {
                    throw CompilerError("Invalid operation with string");
                }
            }
        }
    }
}

std::shared_ptr<syntax::Expr> Scope::simplify(std::shared_ptr<BinaryExpr> binary) {
    using type = Constant::Type;
    using kind = syntax::Expr::Kind;

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
        return std::make_unique<BinaryExpr>(
            left,
            right,
            op,
            begin,
            end
        );
    }

    auto cl = std::static_pointer_cast<Constant>(left);
    auto cr = std::static_pointer_cast<Constant>(right);

    if (cl->type() == type::null || cr->type() == type::null) {
        throw CompilerError("Invalid operation with null")
            .push(binary->begin(), "expression");
    }

    switch (cl->type()) {
        case type::integer:
            switch (cr->type()) {
                case type::integer:
                case type::boolean:
                    return operate<int, int>(cl.get(), op, cr.get());
                default:
                    return operate<std::string, std::string>(cl.get(), op, cr.get());
            }
        case type::boolean:
            switch (cr->type()) {
                case type::integer:
                case type::boolean:
                    return operate<bool, bool>(cl.get(), op, cr.get());
                default:
                    return operate<std::string, std::string>(cl.get(), op, cr.get());
            }
        // type::string
        default:
            return operate<std::string, std::string>(cl.get(), op, cr.get());
    }

    return std::make_unique<BinaryExpr>(left, right, op, begin, end);
}
