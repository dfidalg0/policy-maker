#ifndef __SEMANTICS_SCOPE_HH__
#define __SEMANTICS_SCOPE_HH__

#include <unordered_map>
#include <string>

#include "symbols/_symbol.hh"

class Scope {
public:
    Scope(Scope* parent = nullptr) : _parent(parent){};

    Scope add(Symbol* symbol);
    Symbol* find(std::string name);
private:
    Scope * _parent;
    std::unordered_map<std::string, Symbol*> _symbols;
};

#endif // __SEMANTICS_SCOPE_HH__
