#ifndef __SYNTAX_NODES_IMPORT_STMT_HH__
#define __SYNTAX_NODES_IMPORT_STMT_HH__

#include "_stmt.hh"
#include "import_arg.hh"
#include <string>
#include <vector>
#include <memory>

namespace syntax {
    typedef std::vector<std::shared_ptr<ImportArg>> ImportList;

    class ImportStmt : public Stmt {
    public:
        ImportStmt(
            std::string module,
            std::shared_ptr<ImportList> imports,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Stmt(Node::Kind::import_stmt, begin, end), _module(module), _imports(imports) {}

        inline std::string module() const { return _module; }
        inline ImportList imports() const { return *_imports; }

        void print(uint indent = 0) override;
    private:
        std::string _module;
        std::shared_ptr<ImportList> _imports;
    };
}

#endif // __SYNTAX_NODES_IMPORT_STMT_HH__
