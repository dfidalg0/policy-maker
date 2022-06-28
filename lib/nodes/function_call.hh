#ifndef __FUNCTION_CALL_HH__
#define __FUNCTION_CALL_HH__

#include <string>
#include <vector>
#include <nodes/_expr.hh>

class FunctionCall: public Expr {
private:
    std::string _name;
    std::vector<Expr*> * _args;
public:
    FunctionCall(Position begin, Position end, std::string name, std::vector<Expr*> * args)
        : Expr(Node::Kind::function_call, begin, end), _name(name), _args(args) {}

    inline std::string name() const { return _name; }
    inline std::vector<Expr*> args() const { return *_args; }

    void print(uint indent = 0) override;

    ~FunctionCall() override;
};

#endif // __FUNCTION_CALL_HH__
