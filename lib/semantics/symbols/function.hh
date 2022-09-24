#ifndef __SEMANTICS_SYMBOLS_FUNCTION_HH__
#define __SEMANTICS_SYMBOLS_FUNCTION_HH__

#include <string>
#include <vector>
#include <syntax/nodes/function_decl.hh>

#include "_symbol.hh"
#include "../scope.hh"

namespace semantics {
    class Function : public Symbol {
    public:
        Function(FunctionDecl * decl, Scope * scope);
        Expr * call(std::vector<Expr*> args);
        virtual ~Function(){};
    private:
        Expr * _body;
        Scope * _decl_scope;
        std::vector<std::string> _args;

        void validate(Expr * expr = nullptr);
    };
};

#endif // __SEMANTICS_SYMBOLS_FUNCTION_HH__
