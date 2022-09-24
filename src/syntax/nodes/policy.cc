#include <syntax/nodes/policy.hh>

Policy::~Policy() {
    for (auto rule : *_rules) {
        delete rule;
    }

    delete _rules;
}

void Policy::print(uint level) {
    std::cout << indent(level) << "> Policy Declaration: " << std::endl;

    std::cout << indent(level + 2) << "- Name: " << _name << std::endl;

    if (_rules->size()) {
        std::cout << indent(level + 2) << "- Rules: " << std::endl;
        for (auto rule : *_rules) {
            rule->print(level + 4);
        }
    }
}
