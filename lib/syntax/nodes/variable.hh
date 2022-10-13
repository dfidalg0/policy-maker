#ifndef __SYNTAX_NODES_VARIABLE_HH__
#define __SYNTAX_NODES_VARIABLE_HH__

#include "_expr.hh"

class Variable : public Expr {
private:
    std::string _name;

public:
    Variable(
        std::string name,
        Position begin = Position(0, 0),
        Position end = Position(0, 0)
    ) : Expr(Node::Kind::variable, begin, end), _name(name) {}

    inline std::string name() const { return _name; }

    void print(uint level = 0) override;
};

#endif // __SYNTAX_NODES_VARIABLE_HH__
