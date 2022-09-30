#include <semantics.hh>
#include <syntax/nodes/function_decl.hh>
#include <syntax/nodes/variable_decl.hh>

AnalysisResult * analyze(std::string filename) {
    Program * prog = parse(filename.c_str());
    return analyze(prog);
}

AnalysisResult * analyze(Program *prog) {
    // Primeiro, definimos o escopo global
    auto scope = new Scope();

    using kind = Node::Kind;

    for (auto stmt : prog->stmts()) {
        // E o atualizamos com as declarações globais
        switch (stmt->kind()) {
            case kind::function_decl: {
                auto func_decl = (FunctionDecl *) stmt;
                auto func = new semantics::Function(func_decl, scope);
                scope->add(func);
                break;
            }
            case kind::variable_decl: {
                auto var_decl = (VariableDecl *) stmt;

                auto name = var_decl->name();
                auto value = scope->evaluate(var_decl->value());

                if (value->kind() != kind::constant) {
                    throw std::runtime_error("Global variables must be constant");
                }

                auto var = new semantics::Variable(name, value);

                scope->add(var);
                break;
            }
        }
    }

    auto policies = new Policies();

    for (auto stmt : prog->stmts()) {
        if (stmt->kind() != kind::policy) continue;

        auto policy = (Policy *) stmt;
        auto name = policy->name();
        auto rules = policy->rules();

        auto policy_rules = new PolicyRules();
        policies->insert({name, policy_rules});

        for (auto &rule : rules) {
            for (auto &syscall : rule->syscalls()) {
                auto name = syscall->name();
                auto condition = syscall->condition();

                SyscallRules *syscall_rules;

                auto it = policy_rules->find(name);

                if (it == policy_rules->end()) {
                    syscall_rules = new SyscallRules();
                    policy_rules->insert({ name, syscall_rules });
                } else {
                    syscall_rules = it->second;
                }

                auto evaluated_condition = scope->evaluate(condition);

                syscall_rules->push_back({
                    evaluated_condition,
                    rule->action()
                });
            }
        }
    }

    return new AnalysisResult(policies, scope);
}
