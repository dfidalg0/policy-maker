#ifndef __SYNTAX_NODES_PROGRAM_HH__
#define __SYNTAX_NODES_PROGRAM_HH__

#include "_node.hh"
#include "_stmt.hh"
#include <vector>
#include <memory>

namespace syntax {
    typedef std::vector<std::shared_ptr<Stmt>> StmtList;

    class Program : public Node {
    private:
        std::shared_ptr<StmtList> _stmts;
        std::string _filename;
    public:
        Program(
            std::string filename,
            std::shared_ptr<StmtList> stmts,
            Position begin = Position(0 , 0),
            Position end = Position(0, 0)
        ) : Node(Node::Kind::program, begin, end), _filename(filename), _stmts(stmts) {};

        inline StmtList& stmts() { return *_stmts; }

        inline std::string& filename() { return _filename; }

        void print(uint level = 0) override;
    };
}


#endif // __SYNTAX_NODES_PROGRAM_HH__
