#ifndef __STMT_HH__
#define __STMT_HH__

#include <nodes/_node.hh>

class Stmt : public Node {
public:
    Stmt(Node::Kind kind, Position begin, Position end) : Node(kind, begin, end) {};
    virtual ~Stmt() {};

    virtual void print(uint indent = 0) = 0;
};

#endif // __STMT_HH__
