#ifndef __IDENTIFIER_HH__
#define __IDENTIFIER_HH__

#include <nodes/_expr.hh>

class Variable : public Expr {
private:
    std::string _name;

public:
    Variable(std::string name, Position begin, Position end)
        : Expr(Node::Kind::identifier, begin, end), _name(name) {}

    ~Variable() {}

    inline std::string name() const { return _name; }

    void print(uint indent = 0) override;
};

#endif // __IDENTIFIER_HH__
