#ifndef __SEMANTICS_SCOPE_HH__
#define __SEMANTICS_SCOPE_HH__

#include <unordered_map>
#include <string>
#include <memory>

#include <syntax/nodes.hh>

#include "symbols/_symbol.hh"

class Scope {
public:
    Scope(std::shared_ptr<Scope> parent = nullptr) : _parent(parent){};

    Scope& add(std::shared_ptr<Symbol> symbol);
    std::shared_ptr<Symbol> find(std::string name);

    std::shared_ptr<Expr> evaluate(std::shared_ptr<Expr> expr);
    inline const std::unordered_map<std::string, std::shared_ptr<Symbol>> &symbols() const {
        return _symbols;
    }
private:
    std::shared_ptr<Scope> _parent;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> _symbols;

    std::shared_ptr<Expr> simplify(std::shared_ptr<UnaryExpr> unary);
    std::shared_ptr<Expr> simplify(std::shared_ptr<BinaryExpr> binary);
};

#endif // __SEMANTICS_SCOPE_HH__
