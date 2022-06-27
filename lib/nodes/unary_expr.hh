#ifndef __UNARY_EXPR_HH__
#define __UNARY_EXPR_HH__

#include <nodes/_expr.hh>
#include <string>
#include <exception>
#include <token.hh>

class UnaryExpr : public Expr {
public:
    enum class OpKind {
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
    Expr * _operand;
    OpKind _op;
public:
    UnaryExpr(Expr * operand, OpKind op, Position begin, Position end)
        : Expr(Node::Kind::unary_expr, begin, end), _operand(operand), _op(op) {}

    inline Expr * operand() const { return _operand; }
    inline OpKind op() const { return _op; }

    void print(uint indent = 0) override;

    ~UnaryExpr() override;
};

#endif // __UNARY_EXPR_HH__
