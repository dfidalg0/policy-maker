#ifndef __NODE_HH__
#define __NODE_HH__

#include <iostream>
#include <lexicon/position.hh>
#include <string>

class Node {
public:
    enum class Kind {
        // Root
        program,
        // Building blocks
        action,
        syscall,
        rule,
        // Statements
        policy,
        function_decl,
        variable_decl,
        // Expressions
        unary_expr,
        binary_expr,
        constant,
        syscall_param,
        variable,
        function_call,
    };

    Node(Node::Kind kind, Position begin, Position end) : _kind(kind), _begin(begin), _end(end) {}

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

#endif
