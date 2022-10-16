#ifndef __SYNTAX_NODES__STMT_HH__
#define __SYNTAX_NODES__STMT_HH__

#include "_node.hh"
#include <vector>
#include <memory>

namespace syntax {
    class Stmt : public Node {
    public:
        Stmt(
            Node::Kind kind,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Node(kind, begin, end) {};
        virtual ~Stmt() {};

        virtual void print(uint level = 0) = 0;
    };

    typedef std::vector<std::shared_ptr<Stmt>> StmtList;
};

#endif // __SYNTAX_NODES__STMT_HH__
