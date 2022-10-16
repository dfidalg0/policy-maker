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
    std::cout << indent(level + 2) << "- Type: " << type_to_string(_type) << '\n';
    std::cout << indent(level + 2) << "- Value: ";

    switch (_type) {
        case Type::integer:
            std::cout << *(int *) _value;
            break;
        case Type::string: {
            auto str = *(std::string *) _value;

            std::cout << '"';

            for (auto c: str) {
                switch (c) {
                    case '\n': std::cout << "\\n"; break;
                    case '\t': std::cout << "\\t"; break;
                    case '\r': std::cout << "\\r"; break;
                    case '\v': std::cout << "\\v"; break;
                    case '\f': std::cout << "\\f"; break;
                    case '\b': std::cout << "\\b"; break;
                    case '\\': std::cout << "\\\\"; break;
                    case '\'': std::cout << "\\'"; break;
                    case '"': std::cout << "\\\""; break;
                    default: std::cout << c;
                }
            }

            std::cout << '"';

            break;
        }
        case Type::boolean:
            std::cout << *(bool *) _value;
            break;
        case Type::null:
            std::cout << "null";
            break;
    }

    std::cout << '\n';
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
