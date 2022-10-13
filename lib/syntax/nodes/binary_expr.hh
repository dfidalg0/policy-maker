#ifndef __BINARY_EXPR_HH__
#define __BINARY_EXPR_HH__

#include <exception>
#include <lexicon/token.hh>
#include <string>
#include <memory>

#include "_expr.hh"

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
    std::shared_ptr<Expr> _left;
    std::shared_ptr<Expr> _right;
    OpKind _op;
public:
    BinaryExpr(
        std::shared_ptr<Expr> left,
        std::shared_ptr<Expr> right,
        OpKind op,
        Position begin = Position(0, 0),
        Position end = Position(0, 0)
    ) : Expr(Node::Kind::binary_expr, begin, end), _left(left), _right(right), _op(op) {}

    inline std::shared_ptr<Expr> left() const { return _left; }
    inline std::shared_ptr<Expr> right() const { return _right; }
    inline OpKind op() const { return _op; }

    void print(uint level = 0) override;
};

#endif // __BINARY_EXPR_HH__
