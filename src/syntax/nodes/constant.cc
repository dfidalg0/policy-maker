#include <syntax/nodes/constant.hh>

using namespace syntax;

Constant::~Constant() {
    switch (_type) {
        case Type::integer:
            delete (int *) _value;
            break;
        case Type::string:
            delete (std::string *) _value;
            break;
        case Type::boolean:
            delete (bool *) _value;
            break;
        case Type::null:
            break;
    }
}

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

Constant::operator int() {
    switch (_type) {
        case Type::integer: return *(int *) _value;
        case Type::string: return std::stoi((std::string) * this);
        case Type::boolean: return (int) (bool) * this;
        case Type::null: return 0;
        default: throw std::runtime_error("Never reached");
    }
}

Constant::operator std::string() {
    switch (_type) {
        case Type::string: return *(std::string *) _value;
        case Type::integer: return std::to_string((int) *this);
        case Type::boolean: return (bool) *this ? "true" : "false";
        case Type::null: return "null";
        default: throw std::runtime_error("Never reached");
    }
}

Constant::operator bool() {
    switch (_type) {
        case Type::boolean: return *(bool *) _value;
        case Type::integer: return (bool) (int) *this;
        case Type::string: return (std::string) *this != "";
        case Type::null: return false;
        default: throw std::runtime_error("Never reached");
    }
}

bool Constant::is_truthy() {
    return (bool) *this;
}
