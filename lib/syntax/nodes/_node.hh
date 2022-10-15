#ifndef __SYNTAX_NODES__NODE_HH__
#define __SYNTAX_NODES__NODE_HH__

#include <iostream>
#include <lexicon/position.hh>
#include <string>

namespace syntax {
    class Node {
    public:
        enum class Kind {
            // Root
            program,
            // Building blocks
            action,
            syscall,
            rule,
            import_arg,
            // Statements
            policy,
            function_decl,
            variable_decl,
            import_stmt,
            // Expressions
            unary_expr,
            binary_expr,
            constant,
            syscall_param,
            variable,
            function_call,
        };

        Node(
            Node::Kind kind,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : _kind(kind), _begin(begin), _end(end) {}

        inline Position begin() const { return _begin; }
        inline Position end() const { return _end; }
        inline Node::Kind kind() const { return _kind; }

        static std::string indent(uint indent);

        virtual void print(uint level = 0) = 0;

        virtual ~Node() {}

    private:
        Position _begin;
        Position _end;
        Node::Kind _kind;
    };
};

#endif // __SYNTAX_NODES__NODE_HH__
