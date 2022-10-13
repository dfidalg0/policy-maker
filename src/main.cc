#include <iostream>
#include <syntax/main.hh>
#include <semantics/main.hh>
#include <compile.hh>
#include <utils/cmd_parse.hh>
#include <unistd.h>
#include <cstring>

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>

using std::cout;
using std::endl;
using std::cerr;

int main(int argc, char const * argv[]) {
    auto cmd_parser = utils::CmdParser()
        .add_help_arg()
        .add_arg({
            .name = "input",
            .type = utils::ArgType::STRING,
            .required = true,
            .positional = true,
            .description = "The input file to compile",
            .shorthand = "i",
        })
        .add_arg({
            .name = "print-ast",
            .type = utils::ArgType::NONE,
            .required = false,
            .positional = false,
            .description = "Print the AST to stdout",
        })
        .add_arg({
            .name = "print-analyzed-ast",
            .type = utils::ArgType::NONE,
            .required = false,
            .positional = false,
            .description = "Print the analyzed AST to stdout",
        })
        .add_arg({
            .name = "print-bpf",
            .type = utils::ArgType::NONE,
            .required = false,
            .positional = false,
            .description = "Print the final BPF code to stdout",
        })
        .add_arg({
            .name = "entry",
            .type = utils::ArgType::STRING,
            .required = false,
            .positional = false,
            .description = "The entry point of the program",
            .shorthand = "e",
            .default_value = "main",
        });

    try {
        auto options = cmd_parser.parse(argc, argv);

        auto ast = parse(options.get("input")).release();

        if (options.has("print-ast")) {
            cout << "AST:" << endl;
            ast->print();
        }

        auto result = analyze(ast).release();

        delete ast;

        if (options.has("print-analyzed-ast")) {
            cout << "Analyzed AST:" << endl;

            auto policies = result->policies();
            auto symbols = result->scope()->symbols();

            cout << "Symbols:" << endl;

            for (auto [name, symbol] : symbols) {
                cout << "  - " << name;

                if (symbol->kind() == Symbol::Kind::variable) {
                    auto var = std::static_pointer_cast<semantics::Variable>(symbol);
                    cout << ": Variable" << endl;
                    var->value()->print(4);
                }
                else {
                    auto fn = std::static_pointer_cast<semantics::Function>(symbol);
                    cout << ": Function" << endl;
                    cout << "    - Args: " << endl;
                    for (auto arg : fn->args()) {
                        cout << "        - " << arg << endl;
                    }
                    cout << "    - Body:" << endl;
                    fn->body()->print(8);
                }
            }

            cout << "Policies:" << endl;

            for (auto & policy : *policies) {
                cout << "  - Policy: " << policy.first << endl;

                cout << "    Default action: ";

                auto default_action = policy.second->default_action();

                cout << Action::kind_to_string(default_action->action_kind());

                if (default_action->param() != -1) {
                    cout << "(" << default_action->param() << ")";
                }

                cout << endl;

                for (auto & syscall : *policy.second->rules()) {
                    cout << "    Syscall: " << syscall.first << endl;

                    for (auto & rule : *syscall.second) {
                        cout
                            << "      Action: "
                            << Action::kind_to_string(rule.second->action_kind());

                        if (rule.second->param() != -1) {
                            cout << "(" << rule.second->param() << ")";
                        }

                        if (rule.first) {
                            cout << " IF\n";
                            rule.first->print(8);
                        }
                        else {
                            std::cout << endl;
                        }
                    }
                }
            }
        }

        auto compile_result = compile(result, options.get("entry"));

        delete result;

        sock_fprog prog = compile_result;

        if (options.has("print-bpf")) {
            std::string prog_str = compile_result;

            cout << "Compiled program:\n" << endl;

            cout << prog_str << endl;
        }

        if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
            perror("prctl");
            return 1;
        }

        if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
            perror("seccomp");
            return 1;
        }

        auto _rest = options.rest();

        auto size = _rest.size();

        if (!size) {
            cout << "Code compiled successfully" << endl;
            return 0;
        }

        char ** _argv = new char*[size + 1];

        for (uint i = 0; i < size; ++i) {
            _argv[i] = new char[strlen(_rest[i]) + 1];
            strcpy(_argv[i], _rest[i]);
        }

        _argv[size] = nullptr;

        execvp(_argv[0], (char * const *) _argv);
    }
    catch (FileNotFoundError e) {
        cerr << e.what() << endl;
        return 1;
    }
    catch(ParseError e) {
        cerr << e.what() << endl;
        return 2;
    }
    catch (std::runtime_error e) {
        cerr << e.what() << endl;
        return 3;
    }
    catch (...) {
        cerr << "Unknown error" << endl;
        return -1;
    }

    return 0;
}
