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
                .index = i,
                .param = param
            });
        }
    }

    return params;
}

std::unique_ptr<AnalysisResult> analyze(std::string filename) {
    Program * prog = parse(filename).get();
    return analyze(prog);
}

std::unique_ptr<AnalysisResult> analyze(Program *prog) {
    // Primeiro, definimos o escopo global
    auto global_scope = std::make_shared<Scope>();

    using kind = Node::Kind;

    for (auto stmt : prog->stmts()) {
        // E o atualizamos com as declarações globais
        switch (stmt->kind()) {
            case kind::function_decl: {
                auto func_decl = std::static_pointer_cast<FunctionDecl>(stmt);
                auto func = std::make_shared<semantics::Function>(func_decl.get(), global_scope);
                global_scope->add(func);
                break;
            }
            case kind::variable_decl: {
                auto var_decl = std::static_pointer_cast<VariableDecl>(stmt);

                auto name = var_decl->name();
                auto value = global_scope->evaluate(var_decl->value());

                if (value->kind() != kind::constant) {
                    throw std::runtime_error("Global variables must be constant");
                }

                auto var = std::make_shared<semantics::Variable>(name, value);

                global_scope->add(var);
                break;
            }
        }
    }

    auto policies = std::make_shared<Policies>();

    for (auto stmt : prog->stmts()) {
        if (stmt->kind() != kind::policy) continue;

        auto policy_stmt = std::static_pointer_cast<Policy>(stmt);
        auto name = policy_stmt->name();
        auto rules = policy_stmt->rules();

        auto policy_rules = std::make_shared<PolicyRules>();

        auto policy = std::make_shared<AnalysisResultPolicy>(
            policy_rules,
            policy_stmt->default_action()
        );

        policies->insert({name, policy});

        for (auto &rule : rules) {
            for (auto &syscall : rule->syscalls()) {
                auto name = syscall->name();
                auto condition = syscall->condition();

                auto entry = get_syscall_entry(name);

                auto & nr = entry.nr;
                auto & overloads = entry.overloads;

                auto syscall_rules = ([&policy_rules, &nr]() {
                    auto syscall_rules = std::make_shared<SyscallRules>();

                    auto it = policy_rules->find(nr);

                    if (it == policy_rules->end()) {
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
                        std::make_shared<semantics::SyscallParam>(
                            ":" + param.name, index, param.pointer
                        )
                    );
                }

                std::shared_ptr<Expr> evaluated_condition = scope->evaluate(condition);

                syscall_rules->push_back({
                    evaluated_condition,
                    rule->action()
                });

                delete scope;
            }
        }
    }

    return std::make_unique<AnalysisResult>(policies, global_scope);
}
