#include <syntax/nodes/_node.hh>

std::string Node::indent(uint indent) {
    return std::string(indent, ' ');
}
