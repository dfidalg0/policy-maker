#ifndef __SEMANTICS_SYMBOLS_VARIABLE_HH__
#define __SEMANTICS_SYMBOLS_VARIABLE_HH__

#include <string>

#include "_symbol.hh"

namespace semantics {
    class Variable : public Symbol {
    public:
        Variable(std::string name) : Symbol(Symbol::Kind::variable, name){};
        virtual ~Variable(){};
    };
};

#endif // __SEMANTICS_SYMBOLS_VARIABLE_HH__
