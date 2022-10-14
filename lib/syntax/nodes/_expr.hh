#ifndef __SYNTAX_NODES__EXPR_HH__
#define __SYNTAX_NODES__EXPR_HH__

#include "_node.hh"

namespace syntax {
    class Expr : public Node {
    public:
        Expr(
            Node::Kind kind,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Node(kind, begin, end){};
        virtual ~Expr(){};

        virtual void print(uint level = 0) = 0;
    };
};

#endif  // __SYNTAX_NODES__EXPR_HH__
