#ifndef __FUNCTION_DECL_HH__
#define __FUNCTION_DECL_HH__

#include <nodes/_stmt.hh>
#include <nodes/_expr.hh>
#include <string>
#include <vector>

class FunctionDecl: public Stmt {
private:
    std::string _name;
    std::vector<std::string> * _args;
    Expr * _body;
public:
    FunctionDecl(Position begin, Position end, std::string name, std::vector<std::string> * args, Expr * body)
        : Stmt(Node::Kind::function_decl, begin, end), _name(name), _args(args), _body(body) {}

    ~FunctionDecl();

    inline std::string name() const { return _name; }
    inline std::vector<std::string> args() const { return *_args; }
    inline Expr* body() const { return _body; }

    void print(uint indent = 0) override;
};

#endif // __FUNCTION_DECL_HH__
