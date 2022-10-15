#ifndef __SYNTAX_NODES_IMPORT_ARG_HH__
#define __SYNTAX_NODES_IMPORT_ARG_HH__

#include "_node.hh"

namespace syntax {
    class ImportArg: public Node {
    public:
        inline std::string name() const { return _name; }
        inline std::string alias() const { return _aliased ? _alias : _name; }
        inline bool aliased() const { return _aliased; }
        inline bool is_policy() const { return _policy; }

        static ImportArg * Symbol(
            std::string name,
            Position begin = Position(0, 0),
            Position(end) = Position(0, 0)
        ) {
            return new ImportArg(name, false, begin, end);
        }

        static ImportArg * Symbol(
            std::string name,
            std::string alias,
            Position begin = Position(0, 0),
            Position(end) = Position(0, 0)
        ) {
            return new ImportArg(name, alias, false, begin, end);
        }

        static ImportArg * Policy(
            std::string name,
            Position begin = Position(0, 0),
            Position(end) = Position(0, 0)
        ) {
            return new ImportArg(name, true, begin, end);
        }

        static ImportArg * Policy(
            std::string name,
            std::string alias,
            Position begin = Position(0, 0),
            Position(end) = Position(0, 0)
        ) {
            return new ImportArg(name, alias, true, begin, end);
        }

        void print(uint level = 0) override;
    private:
        ImportArg(
            std::string name,
            bool policy,
            Position begin,
            Position end
        ): Node(Kind::import_arg, begin, end), _name(name), _aliased(false), _policy(policy) {}

        ImportArg(
            std::string name,
            std::string alias,
            bool policy,
            Position begin,
            Position end
        ) : Node(Kind::import_arg, begin, end), _name(name), _alias(alias), _aliased(true), _policy(policy) {}

        std::string _name;
        std::string _alias;
        bool _aliased;
        bool _policy;
    };
}

#endif // __SYNTAX_NODES_IMPORT_ARG_HH__
