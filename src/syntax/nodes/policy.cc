#include <syntax/nodes/policy.hh>

using namespace syntax;

void Policy::print(uint level) {
    std::cout << indent(level) << "> Policy Declaration: " << std::endl;

    std::cout << indent(level + 2) << "- Name: " << _name << std::endl;

    std::cout << indent(level + 2) << "- Default action: " << std::endl;
    _default_action->print(level + 4);

    if (_body->size()) {
        std::cout << indent(level + 2) << "- Rules: " << std::endl;
        for (auto rule : *_body) {
            rule->print(level + 4);
        }
    }
}
