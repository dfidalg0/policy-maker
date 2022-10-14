#ifndef __SYNTAX_NODES_VARIABLE_DECL_HH__
#define __SYNTAX_NODES_VARIABLE_DECL_HH__

#include <memory>

#include "_stmt.hh"
#include "_expr.hh"

namespace syntax {
    class VariableDecl : public Stmt {
    private:
        std::string _name;
        std::shared_ptr<Expr> _value;

    public:
        VariableDecl(
            std::string name,
            std::shared_ptr<Expr> value,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Stmt(Node::Kind::variable_decl, begin, end), _name(name), _value(value) {}

        inline std::string name() const { return _name; }

        inline std::shared_ptr<Expr>  value() const { return _value; }

        void print(uint level = 0) override;
    };
};

#endif // __SYNTAX_NODES_VARIABLE_DECL_HH__
