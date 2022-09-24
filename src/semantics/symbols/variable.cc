#include <semantics/symbols/variable.hh>

using namespace semantics;

Variable::Variable(std::string name, Expr * value)
    : Symbol(Symbol::Kind::variable, name), _value(value) {}
