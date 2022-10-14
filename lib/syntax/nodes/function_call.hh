#ifndef __SYNTAX_NODES_FUNCTION_CALL_HH__
#define __SYNTAX_NODES_FUNCTION_CALL_HH__

#include <string>
#include <vector>
#include <memory>
#include "_expr.hh"

namespace syntax {
    typedef std::vector<std::shared_ptr<Expr>> ExprList;

    class FunctionCall: public Expr {
    private:
        std::string _name;
        std::shared_ptr<ExprList> _args;
    public:
        FunctionCall(
            std::string name,
            std::shared_ptr<ExprList> args,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Expr(Node::Kind::function_call, begin, end), _name(name), _args(args) {}

        inline std::string name() const { return _name; }
        inline ExprList& args() const { return *_args; }

        void print(uint level = 0) override;
    };
}


#endif // __SYNTAX_NODES_FUNCTION_CALL_HH__
