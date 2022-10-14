#include <syntax/nodes/unary_expr.hh>

using namespace syntax;

void UnaryExpr::print(uint level) {
    std::cout << indent(level) << "> Unary Expression" << std::endl;
    std::cout << indent(level + 2) << "- Operator: " << kind_to_string(_op) << std::endl;
    _operand->print(level + 2);
}

UnaryExpr::OpKind UnaryExpr::kind_from_token(Token * token) {
    switch (token->type()) {
        case yytokentype::SUB: return OpKind::neg;
        case yytokentype::ADD: return OpKind::pos;
        case yytokentype::NOT: return OpKind::bit_not;
        case yytokentype::LOGICAL_NOT: return OpKind::lnot;
        default: throw UnaryExpr::Exception("Unknown operator");
    }
}

std::string UnaryExpr::kind_to_string(OpKind kind) {
    switch (kind) {
        case OpKind::neg:
            return "Negative";
        case OpKind::pos:
            return "Positive";
        case OpKind::bit_not:
            return "Bitwise Not";
        case OpKind::lnot:
            return "Logical Not";
        default:
            return "unknown";
    }
}
