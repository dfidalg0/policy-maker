#ifndef __PROGRAM_HH__
#define __PROGRAM_HH__

#include <nodes/_node.hh>
#include <nodes/_stmt.hh>
#include <vector>

class Program : public Node {
private:
    std::vector<Stmt *> * _stmts;
public:
    Program(
        Position begin,
        Position end,
        std::vector<Stmt *> * stmts
    ) : Node(Node::Kind::program, begin, end), _stmts(stmts) {};

    ~Program();

    inline std::vector<Stmt *> stmts() { return *_stmts; }

    void print(uint level = 0) override;
};

#endif // __PROGRAM_HH__
