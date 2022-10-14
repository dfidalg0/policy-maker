#include <semantics/symbols/variable.hh>

using namespace semantics;

Variable::Variable(std::string name, std::shared_ptr<syntax::Expr> value)
    : Symbol(Symbol::Kind::variable, name), _value(value) {}
