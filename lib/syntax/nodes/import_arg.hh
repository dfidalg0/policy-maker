#ifndef __SYNTAX_NODES_IMPORT_ARG_HH__
#define __SYNTAX_NODES_IMPORT_ARG_HH__

#include "_node.hh"

namespace syntax {
    class ImportArg: public Node {
    public:
        ImportArg(
            std::string name,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ): Node(Kind::import_arg, begin, end), _name(name), _aliased(false) {
            _alias = name;
        }

        ImportArg(
            std::string name,
            std::string alias,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Node(Kind::import_arg, begin, end), _name(name), _alias(alias), _aliased(true) {}

        inline std::string name() const { return _name; }
        inline std::string alias() const { return _alias; }
        inline bool aliased() const { return _aliased; }

        void print(uint level = 0) override;
    private:
        std::string _name;
        std::string _alias;
        bool _aliased;
    };
}

#endif // __SYNTAX_NODES_IMPORT_ARG_HH__
