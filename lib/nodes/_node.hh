#ifndef __NODE_HH__
#define __NODE_HH__

#include <position.hh>
#include <string>
#include <iostream>

class Node {
public:
    enum class Kind {
        action,
        syscall,
        rule,
        policy,
        program,
    };

    Node(Node::Kind kind, Position begin, Position end) : _kind(kind), _begin(begin), _end(end) {}

    inline Position begin() const { return _begin; }
    inline Position end() const { return _end; }
    inline Node::Kind kind() const { return _kind; }

    static std::string indent(uint indent);

    virtual void print(uint indent = 0) = 0;

private:
    Position _begin;
    Position _end;
    Node::Kind _kind;
};

#endif
