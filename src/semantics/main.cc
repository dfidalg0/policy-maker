#include <semantics.hh>
#include <syntax/nodes/function_decl.hh>
#include <syntax/nodes/variable_decl.hh>
#include <parser.yy.hh>
#include <errors.hh>
#include <filesystem>
#include <set>

#include <syscalls.hh>

using namespace semantics;

namespace fs = std::filesystem;

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

std::unique_ptr<AnalysisResult> semantics::analyze(std::string filename) {
    auto prog = syntax::parse(filename).release();
    auto ar = analyze(prog);
    delete prog;
    return ar;
}

std::unique_ptr<AnalysisResult> semantics::analyze(syntax::Program *prog) {
    // Primeiro, definimos o escopo global
    auto global_scope = std::make_shared<semantics::Scope>();

    auto file = fs::path(prog->filename());
    auto dir = file.parent_path();

    using kind = syntax::Node::Kind;

    auto policies = std::make_shared<semantics::Policies>();

    auto add_policy = [&policies] (std::string name, std::shared_ptr<semantics::Policy> policy) {
        auto it = policies->find(name);

        if (it != policies->end()) {
            throw CompilerError("Policy " + name + " already defined");
        }

        policies->insert({name, policy});
    };

    for (auto stmt : prog->stmts()) {
        // E o atualizamos com as declarações globais
        switch (stmt->kind()) {
            case kind::import_stmt: {
                auto import = std::static_pointer_cast<syntax::ImportStmt>(stmt);

                auto path = fs::weakly_canonical(dir / import->module());

                if (!fs::exists(path)) {
                    throw CompilerError("Module not found: " + import->module())
                        .push(import->begin(), "import statement")
                        .build(file);
                }

                auto ar = analyze(path.string());
                auto mod_policies = ar->policies();
                auto mod_scope = ar->scope();

                for (auto &arg: import->imports()) {
                    if (arg->is_policy()) {
                        auto pol_it = mod_policies->find(arg->name());

                        if (pol_it == mod_policies->end()) {
                            throw CompilerError("Policy not found: " + arg->name())
                                .push(arg->begin(), "import argument")
                                .push(import->begin(), "import statement")
                                .build(file);
                        }

                        try {
                            add_policy(arg->alias(), pol_it->second);
                            continue;
                        } catch (CompilerError &e) {
                            throw e
                                .push(arg->begin(), "import argument")
                                .push(import->begin(), "import statement")
                                .build(file);
                        }
                    }

                    auto symbol = mod_scope->find(arg->name());

                    if (!symbol) {
                        throw CompilerError("Symbol not found: " + arg->name())
                            .push(arg->begin(), "import argument")
                            .push(import->begin(), "import statement")
                            .build(file);
                    }

                    try {
                        global_scope->add(arg->alias(), symbol);
                    }
                    catch (CompilerError &e) {
                        throw e
                            .push(arg->begin(), "import argument")
                            .push(import->begin(), "import statement")
                            .build(file);
                    }
                }

                break;
            }
            case kind::function_decl: {
                auto func_decl = std::static_pointer_cast<syntax::FunctionDecl>(stmt);

                try {
                    auto func = std::make_shared<semantics::Function>(func_decl.get(), global_scope);

                    global_scope->add(func);
                }
                catch (CompilerError& e) {
                    throw e.build(prog->filename());
                }
                break;
            }
            case kind::variable_decl: {
                auto var_decl = std::static_pointer_cast<syntax::VariableDecl>(stmt);

                auto name = var_decl->name();

                std::shared_ptr<syntax::Expr> value;

                try {
                    value = global_scope->evaluate(var_decl->value());
                }
                catch (CompilerError& e) {
                    throw e
                        .push(var_decl->value()->begin(), "value definition")
                        .push(var_decl->begin(), "variable declaration")
                        .build(prog->filename());
                }

                if (value->kind() != kind::constant) {
                    throw CompilerError("Invalid global variable expression: " + name)
                        .push(value->begin(), "value definition")
                        .push(var_decl->begin(), "variable declaration")
                        .build(prog->filename());
                }

                auto var = std::make_shared<semantics::Variable>(name, value);

                try {
                    global_scope->add(var);
                }
                catch (CompilerError& e) {
                    throw e
                        .push(var_decl->value()->begin(), "variable declaration")
                        .build(prog->filename());
                }

                break;
            }
        }
    }

    for (auto stmt : prog->stmts()) {
        if (stmt->kind() != kind::policy) continue;

        auto policy_stmt = std::static_pointer_cast<syntax::Policy>(stmt);
        auto policy_name = policy_stmt->name();
        auto rules = policy_stmt->rules();

        auto policy_rules = std::make_shared<semantics::PolicyRules>();

        auto policy = std::make_shared<semantics::Policy>(
            policy_rules,
            policy_stmt->default_action()
        );

        try {
            add_policy(policy_name, policy);
        }
        catch (CompilerError& e) {
            throw e
                .push(policy_stmt->begin(), "policy declaration")
                .build(file);
        }

        for (auto &rule : rules) {
            for (auto &syscall : rule->syscalls()) {
                auto sc_name = syscall->name();
                auto condition = syscall->condition();

                gen::SyscallEntry entry;

                try {
                    entry = get_syscall_entry(sc_name);
                }
                catch (CompilerError & e) {
                    auto action_kind = rule->action()->action_kind();
                    auto action_str = syntax::Action::kind_to_string(action_kind);

                    throw e
                        .push(syscall->begin(), "syscall '" + sc_name + "'")
                        .push(rule->begin(), "rule '" + action_str + "'")
                        .push(policy_stmt->begin(), "policy '" + policy_name + "'")
                        .build(prog->filename());
                }

                auto & nr = entry.nr;
                auto & overloads = entry.overloads;

                auto syscall_rules = ([&policy_rules, &nr]() {
                    auto syscall_rules = std::make_shared<semantics::SyscallRules>();

                    auto it = policy_rules->find(nr);

                    if (it == policy_rules->end()) {
                        policy_rules->insert({nr, syscall_rules});
                    } else {
                        syscall_rules = it->second;
                    }

                    return syscall_rules;
                })();

                auto scope = new semantics::Scope(global_scope);

                auto syscall_params = merge_overloads(overloads);

                for (auto &[index, param]: syscall_params) {
                    scope->add(
                        std::make_shared<semantics::SyscallParam>(
                            ":" + param.name, index, param.pointer
                        )
                    );
                }

                std::shared_ptr<syntax::Expr> evaluated_condition;

                try {
                    evaluated_condition = scope->evaluate(condition);
                }
                catch (CompilerError& e) {
                    throw e
                        .push(condition->begin(), "syscall condition")
                        .push(syscall->begin(), "syscall '" + sc_name + "'")
                        .push(rule->begin(), "rule '" + rule->action()->to_string() + "'")
                        .push(policy_stmt->begin(), "policy '" + policy_name + "'")
                        .build(prog->filename());
                }

                syscall_rules->push_back({
                    evaluated_condition,
                    rule->action()
                });

                delete scope;
            }
        }
    }

    return std::make_unique<AnalysisResult>(prog->filename(), policies, global_scope);
}
