#ifndef __SEMANTICS_SCOPE_HH__
#define __SEMANTICS_SCOPE_HH__

#include <unordered_map>
#include <string>
#include <memory>

#include <syntax/nodes.hh>

#include "symbols/_symbol.hh"

namespace semantics {
    class Scope {
    public:
        Scope(std::shared_ptr<Scope> parent) : _parent(parent){
            _file = parent->file();
        };

        Scope(std::string file) : _file(file), _parent(nullptr) {};

        Scope& add(const std::string& name, std::shared_ptr<Symbol> symbol);
        Scope& add(std::shared_ptr<Symbol> symbol);
        std::shared_ptr<Symbol> find(std::string name);

        std::shared_ptr<syntax::Expr> evaluate(std::shared_ptr<syntax::Expr> expr);
        inline const std::unordered_map<std::string, std::shared_ptr<Symbol>> &symbols() const {
            return _symbols;
        }

        inline const std::string &file() const { return _file; }
    private:
        std::shared_ptr<Scope> _parent;
        std::string _file;
        std::unordered_map<std::string, std::shared_ptr<Symbol>> _symbols;

        std::shared_ptr<syntax::Expr> simplify(std::shared_ptr<syntax::UnaryExpr> unary);
        std::shared_ptr<syntax::Expr> simplify(std::shared_ptr<syntax::BinaryExpr> binary);
    };
}

#endif // __SEMANTICS_SCOPE_HH__
