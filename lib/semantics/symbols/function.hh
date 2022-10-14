#ifndef __SEMANTICS_SYMBOLS_FUNCTION_HH__
#define __SEMANTICS_SYMBOLS_FUNCTION_HH__

#include <string>
#include <vector>
#include <memory>
#include <syntax/nodes/function_decl.hh>

#include "_symbol.hh"
#include "../scope.hh"

namespace semantics {
    class Function : public Symbol {
    public:
        Function(syntax::FunctionDecl * decl, std::shared_ptr<Scope> scope);
        std::shared_ptr<syntax::Expr> call(std::vector<std::shared_ptr<syntax::Expr>> args);
        virtual ~Function(){};

        inline const std::vector<std::string> &args() const { return _args; }
        inline std::shared_ptr<syntax::Expr> body() const { return _body; }
    private:
        std::shared_ptr<syntax::Expr> _body;
        std::shared_ptr<Scope> _decl_scope;
        std::vector<std::string> _args;

        void validate(syntax::Expr * expr = nullptr);
    };
};

#endif // __SEMANTICS_SYMBOLS_FUNCTION_HH__
