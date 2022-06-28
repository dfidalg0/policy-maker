#include <nodes/constant.hh>

void Constant::print(uint level) {
    std::cout << indent(level) << "> Constant: \n";
    std::cout << indent(level + 2) << "- Type: " << type_to_string(_type) << std::endl;
    std::cout << indent(level + 2) << "- Value: " << _value << std::endl;
}

std::string Constant::type_to_string(Type type) {
    switch (type) {
        case Type::integer: return "integer";
        case Type::string: return "string";
        case Type::boolean: return "boolean";
        case Type::null: return "null";
        default: return "unknown";
    }
}
