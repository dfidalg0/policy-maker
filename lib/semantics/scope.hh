#ifndef __SEMANTICS_SCOPE_HH__
#define __SEMANTICS_SCOPE_HH__

#include <unordered_map>
#include <string>
#include <syntax/nodes.hh>

#include "symbols/_symbol.hh"

class Scope {
public:
    Scope(Scope* parent = nullptr) : _parent(parent){};

    Scope add(Symbol* symbol);
    Symbol* find(std::string name);

    Expr * evaluate(Expr * expr);
    inline const std::unordered_map<std::string, Symbol *> &symbols() const {
        return _symbols;
    }
private:
    Scope * _parent;
    std::unordered_map<std::string, Symbol*> _symbols;

    Expr * simplify(UnaryExpr *);
    Expr * simplify(BinaryExpr *);
};

#endif // __SEMANTICS_SCOPE_HH__
