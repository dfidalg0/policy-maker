#ifndef __SYNTAX_NODES_FUNCTION_DECL_HH__
#define __SYNTAX_NODES_FUNCTION_DECL_HH__

#include "_stmt.hh"
#include "_expr.hh"
#include <string>
#include <vector>
#include <memory>

class FunctionDecl: public Stmt {
private:
    std::string _name;
    std::shared_ptr<std::vector<std::string>> _args;
    std::shared_ptr<Expr> _body;
public:
    FunctionDecl(
        std::string name,
        std::shared_ptr<std::vector<std::string>> args,
        std::shared_ptr<Expr> body,
        Position begin = Position(0, 0),
        Position end = Position(0, 0)
    ) : Stmt(Node::Kind::function_decl, begin, end), _name(name), _args(args), _body(body) {}

    inline std::string name() const { return _name; }
    inline std::vector<std::string> args() const { return *_args; }
    inline std::shared_ptr<Expr> body() const { return _body; }

    void print(uint level = 0) override;
};

#endif // __SYNTAX_NODES_FUNCTION_DECL_HH__
