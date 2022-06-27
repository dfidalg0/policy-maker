#include <nodes/unary_expr.hh>

UnaryExpr::~UnaryExpr() {
    delete _operand;
}

void UnaryExpr::print(uint indent) {
    std::cout << Node::indent(indent) << "- UnaryExpr" << std::endl;
    std::cout << Node::indent(indent + 2) << "- Operator: " << kind_to_string(_op) << std::endl;
    _operand->print(indent + 2);
}

UnaryExpr::OpKind UnaryExpr::kind_from_token(Token * token) {
    switch (token->type()) {
        case yytokentype::NOT: return OpKind::bit_not;
        case yytokentype::LOGICAL_NOT: return OpKind::lnot;
        default: throw UnaryExpr::Exception("Unknown operator");
    }
}

std::string UnaryExpr::kind_to_string(OpKind kind) {
    switch (kind) {
        case OpKind::bit_not:
            return "Bitwise Not";
        case OpKind::lnot:
            return "Logical Not";
        default:
            return "unknown";
    }
}
