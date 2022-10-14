#include <syntax/nodes/_node.hh>

using namespace syntax;

std::string Node::indent(uint indent) {
    return std::string(indent, ' ');
}
