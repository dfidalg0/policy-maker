#include <semantics.hh>
#include <syntax/nodes/function_decl.hh>
#include <syntax/nodes/variable_decl.hh>
#include <set>

#include <syscalls.hh>

struct SyscallParamWithIndex {
    uint index;
    gen::SyscallParam param;
};

static std::vector<SyscallParamWithIndex> merge_overloads(gen::SyscallOverloads &overloads) {
    std::vector<SyscallParamWithIndex> params;

    std::set<std::string> names;

    for (auto overload : overloads) {
        uint size = overload.size();

        for (uint i = 0; i < size; ++i) {
            auto param = overload[i];

            if (names.find(param.name) != names.end()) continue;

            names.insert(param.name);

            params.push_back({
                .index = i + 1,
                .param = param
            });
        }
    }

    return params;
}

AnalysisResult * analyze(std::string filename) {
    Program * prog = parse(filename.c_str());
    return analyze(prog);
}

AnalysisResult * analyze(Program *prog) {
    // Primeiro, definimos o escopo global
    auto global_scope = new Scope();

    using kind = Node::Kind;

    for (auto stmt : prog->stmts()) {
        // E o atualizamos com as declarações globais
        switch (stmt->kind()) {
            case kind::function_decl: {
                auto func_decl = (FunctionDecl *) stmt;
                auto func = new semantics::Function(func_decl, global_scope);
                global_scope->add(func);
                break;
            }
            case kind::variable_decl: {
                auto var_decl = (VariableDecl *) stmt;

                auto name = var_decl->name();
                auto value = global_scope->evaluate(var_decl->value());

                if (value->kind() != kind::constant) {
                    throw std::runtime_error("Global variables must be constant");
                }

                auto var = new semantics::Variable(name, value);

                global_scope->add(var);
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

                auto entry = get_syscall_entry(name);

                auto & nr = entry.nr;
                auto & overloads = entry.overloads;

                auto syscall_rules = ([&policy_rules, &nr]() {
                    SyscallRules *syscall_rules;

                    auto it = policy_rules->find(nr);

                    if (it == policy_rules->end()) {
                        syscall_rules = new SyscallRules();
                        policy_rules->insert({nr, syscall_rules});
                    } else {
                        syscall_rules = it->second;
                    }

                    return syscall_rules;
                })();

                auto scope = new Scope(global_scope);

                auto syscall_params = merge_overloads(overloads);

                for (auto &[index, param]: syscall_params) {
                    scope->add(
                        new semantics::SyscallParam(
                            ":" + param.name, index, param.pointer
                        )
                    );
                }

                auto evaluated_condition = scope->evaluate(condition);

                syscall_rules->push_back({
                    evaluated_condition,
                    rule->action()
                });

                delete scope;
            }
        }
    }

    return new AnalysisResult(policies, global_scope);
}
