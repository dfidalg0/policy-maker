#ifndef __SEMANTICS_SYMBOLS_VARIABLE_HH__
#define __SEMANTICS_SYMBOLS_VARIABLE_HH__

#include <string>

#include "_symbol.hh"
#include <syntax/nodes/_expr.hh>

namespace semantics {
    class Variable : public Symbol {
    public:
        Variable(std::string name, Expr * value = nullptr);
        virtual ~Variable(){};

        inline Expr * value() const { return _value; };
    private:
        Expr * _value;
    };
};

#endif // __SEMANTICS_SYMBOLS_VARIABLE_HH__
