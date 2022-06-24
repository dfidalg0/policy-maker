#include <nodes/policy.hh>

Policy::~Policy() {
    for (auto rule : *_rules) {
        delete rule;
    }

    delete _rules;
}

void Policy::print(uint indent) {
    std::cout << Node::indent(indent) << "- Policy: " << _name << std::endl;

    if (_rules->size()) {
        std::cout << Node::indent(indent + 2) << "- Rules: " << std::endl;
        for (auto rule : *_rules) {
            rule->print(indent + 4);
        }
    }
}
