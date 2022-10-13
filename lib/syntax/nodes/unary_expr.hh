#ifndef __SYNTAX_NODES_UNARY_EXPR_HH__
#define __SYNTAX_NODES_UNARY_EXPR_HH__

#include <exception>
#include <lexicon/token.hh>
#include <string>
#include <memory>

#include "_expr.hh"

class UnaryExpr : public Expr {
public:
    enum class OpKind {
        // Arithmetic
        neg,
        pos,
        // Bitwise
        bit_not,
        // Logical
        lnot,
    };

    static std::string kind_to_string(OpKind kind);

    static OpKind kind_from_token(Token * token);

    class Exception : public std::exception {
    public:
        Exception(std::string msg) : _msg(msg) {}
        virtual const char* what() const throw() { return _msg.c_str(); }
    private:
        std::string _msg;
    };

private:
    std::shared_ptr<Expr> _operand;
    OpKind _op;
public:
    UnaryExpr(
        std::shared_ptr<Expr> operand,
        OpKind op,
        Position begin = Position(0, 0),
        Position end = Position(0, 0)
    ) : Expr(Node::Kind::unary_expr, begin, end), _operand(operand), _op(op) {}

    inline std::shared_ptr<Expr> operand() const { return _operand; }
    inline OpKind op() const { return _op; }

    void print(uint level = 0) override;
};

#endif // __SYNTAX_NODES_UNARY_EXPR_HH__
