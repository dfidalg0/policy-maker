#ifndef __BINARY_EXPR_HH__
#define __BINARY_EXPR_HH__

#include <nodes/_expr.hh>
#include <token.hh>
#include <string>
#include <exception>

class BinaryExpr : public Expr {
public:
    enum class OpKind {
        // Arithmetic
        add,
        sub,
        mul,
        div,
        mod,
        // Bitwise
        bit_and,
        bit_or,
        bit_xor,
        bit_lsh,
        bit_rsh,
        // Comparison
        eq,
        ne,
        gt,
        lt,
        ge,
        le,
        // Logical
        land,
        lor,
    };

    static std::string kind_to_string(OpKind kind);

    static OpKind kind_from_token(Token * token);

    class Exception : public std::exception {
    public:
        Exception(std::string message) : _message(message) {}
        const char * what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message;
    };
private:
    Expr * _left;
    Expr * _right;
    OpKind _op;
public:
    BinaryExpr(Expr * left, Expr * right, OpKind op, Position begin, Position end)
        : Expr(Node::Kind::binary_expr, begin, end), _left(left), _right(right), _op(op) {}

    inline Expr * left() const { return _left; }
    inline Expr * right() const { return _right; }
    inline OpKind op() const { return _op; }

    void print(uint level = 0) override;

    ~BinaryExpr() override;
};

#endif // __BINARY_EXPR_HH__
