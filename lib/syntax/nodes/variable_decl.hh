#ifndef __VARIABLE_DECL_HH__
#define __VARIABLE_DECL_HH__

#include "_stmt.hh"
#include "_expr.hh"

class VariableDecl : public Stmt {
private:
    std::string _name;
    Expr *_value;

public:
    VariableDecl(std::string name, Expr * value, Position begin, Position end)
        : Stmt(Node::Kind::variable_decl, begin, end), _name(name), _value(value) {}

    ~VariableDecl() {}

    inline std::string name() const { return _name; }

    inline Expr * value() const { return _value; }

    void print(uint level = 0) override;
};

#endif // __VARIABLE_DECL_HH__
