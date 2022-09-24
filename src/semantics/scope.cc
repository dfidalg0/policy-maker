#include <semantics/scope.hh>
#include <stdexcept>

Scope Scope::add(Symbol* symbol) {
    auto name = symbol->name();

    if (_symbols.find(name) != _symbols.end()) {
        throw std::runtime_error("Symbol already defined");
    }

    _symbols[symbol->name()] = symbol;
    return *this;
}

Symbol* Scope::find(std::string name) {
    auto it = _symbols.find(name);

    if (it != _symbols.end())
        return it->second;

    if (_parent != nullptr) return _parent->find(name);

    return nullptr;
}
