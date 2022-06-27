#ifndef __EXPR_HH__
#define __EXPR_HH__

#include <nodes/_node.hh>

class Expr : public Node {
public:
    Expr(Node::Kind kind, Position begin, Position end) : Node(kind, begin, end){};
    virtual ~Expr(){};

    virtual void print(uint indent = 0) = 0;
};

#endif  // __EXPR_HH__
