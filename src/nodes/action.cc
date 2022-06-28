#include <nodes/action.hh>

using tt = yytokentype;

Action::Kind Action::kind_from_token(Token * token) {
    switch (token->type()) {
        case tt::ALLOW: return Kind::allow;
        case tt::KILL: return Kind::kill;
        case tt::TRAP: return Kind::trap;
        case tt::NOTIFY: return Kind::notify;
        case tt::TRACE: return Kind::trace;
        case tt::LOG: return Kind::log;
        case tt::TERMINATE: return Kind::terminate;
        default: throw Action::Exception("Unexpected token type");
    }
}

std::string Action::kind_to_string(Kind kind) {
    switch (kind) {
        case Kind::allow: return "ALLOW";
        case Kind::kill: return "KILL";
        case Kind::trap: return "TRAP";
        case Kind::notify: return "NOTIFY";
        case Kind::trace: return "TRACE";
        case Kind::log: return "LOG";
        case Kind::terminate: return "TERMINATE";
        default: throw Action::Exception("Unexpected action kind");
    }
}

void Action::print(uint level) {
    std::cout << indent(level) << "> Action: " << kind_to_string(_action_kind) << std::endl;
    if (_param != -1) {
        std::cout << indent(level + 2) << "- Param: " << _param << std::endl;
    }
}
