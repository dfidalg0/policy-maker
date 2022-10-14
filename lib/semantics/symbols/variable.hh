#ifndef __SEMANTICS_SYMBOLS_VARIABLE_HH__
#define __SEMANTICS_SYMBOLS_VARIABLE_HH__

#include <string>
#include <syntax/nodes/_expr.hh>
#include <memory>

#include "_symbol.hh"

namespace semantics {
    class Variable : public Symbol {
    public:
        Variable(std::string name, std::shared_ptr<syntax::Expr> value = nullptr);

        inline std::shared_ptr<syntax::Expr> value() const { return _value; };
    private:
        std::shared_ptr<syntax::Expr> _value;
    };
};

#endif // __SEMANTICS_SYMBOLS_VARIABLE_HH__
