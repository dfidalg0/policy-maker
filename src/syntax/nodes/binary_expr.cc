#include <syntax/nodes/binary_expr.hh>

BinaryExpr::~BinaryExpr() {
    delete _left;
    delete _right;
}

void BinaryExpr::print(uint level) {
    std::cout << indent(level) << "> Binary Expression" << std::endl;
    std::cout << indent(level + 2) << "- Operator: " << kind_to_string(_op) << std::endl;
    std::cout << indent(level + 2) << "- Left:" << std::endl;
    _left->print(level + 4);
    std::cout << indent(level + 2) << "- Right:" << std::endl;
    _right->print(level + 4);
}

BinaryExpr::OpKind BinaryExpr::kind_from_token(Token * token) {
    switch (token->type()) {
        case yytokentype::ADD: return OpKind::add;
        case yytokentype::SUB: return OpKind::sub;
        case yytokentype::MUL: return OpKind::mul;
        case yytokentype::DIV: return OpKind::div;
        case yytokentype::MOD: return OpKind::mod;
        case yytokentype::AND: return OpKind::bit_and;
        case yytokentype::OR: return OpKind::bit_or;
        case yytokentype::XOR: return OpKind::bit_xor;
        case yytokentype::LSH: return OpKind::bit_lsh;
        case yytokentype::RSH: return OpKind::bit_rsh;
        case yytokentype::EQ: return OpKind::eq;
        case yytokentype::NE: return OpKind::ne;
        case yytokentype::GT: return OpKind::gt;
        case yytokentype::LT: return OpKind::lt;
        case yytokentype::GE: return OpKind::ge;
        case yytokentype::LE: return OpKind::le;
        case yytokentype::LOGICAL_AND: return OpKind::land;
        case yytokentype::LOGICAL_OR: return OpKind::lor;
        default: throw BinaryExpr::Exception("Unknown operator");
    }
}

std::string BinaryExpr::kind_to_string(OpKind kind) {
    switch (kind) {
        case OpKind::add:
            return "Add";
        case OpKind::sub:
            return "Sub";
        case OpKind::mul:
            return "Multiplication";
        case OpKind::div:
            return "Division";
        case OpKind::mod:
            return "Modulo";
        case OpKind::bit_and:
            return "Bitwise And";
        case OpKind::bit_or:
            return "Bitwise Or";
        case OpKind::bit_xor:
            return "Bitwise Xor";
        case OpKind::bit_lsh:
            return "Left Shift";
        case OpKind::bit_rsh:
            return "Right Shift";
        case OpKind::eq:
            return "Equal";
        case OpKind::ne:
            return "Not Equal";
        case OpKind::lt:
            return "Less Than";
        case OpKind::gt:
            return "Greater Than";
        case OpKind::le:
            return "Less Than or Equal";
        case OpKind::ge:
            return "Greater Than or Equal";
        case OpKind::land:
            return "Logical And";
        case OpKind::lor:
            return "Logical Or";
        default:
            return "unknown";
    }
}
