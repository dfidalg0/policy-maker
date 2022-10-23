#include <iostream>
#include <semantics/main.hh>
#include <compile.hh>
#include <utils/cmd_parse.hh>
#include <unistd.h>
#include <cstring>

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <parser.yy.hh>

using std::cout;
using std::endl;
using std::cerr;

int main(int argc, char const * argv[]) {
    const auto cmd_parser = utils::CmdParser()
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

        auto ast = syntax::parse(options.get("input")).release();

        if (options.has("print-ast")) {
            cout << "AST:" << endl;
            ast->print();
        }

        auto result = semantics::analyze(ast).release();

        delete ast;

        if (options.has("print-analyzed-ast")) {
            cout << "Analyzed AST:\n";
            result->print();
        }

        auto compile_result = compile(result, options.get("entry"));

        delete result;

        sock_fprog prog = compile_result;

        if (options.has("print-bpf")) {
            std::string prog_str = compile_result;

            cout << "Compiled program:\n";

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

        const char ** _argv = new const char*[size + 1];

        for (uint i = 0; i < size; ++i) {
            _argv[i] = _rest[i].c_str();
        }

        _argv[size] = nullptr;

        execvp(_argv[0], (char * const *) _argv);
    }
    catch (FileNotFoundError e) {
        cerr << e.what() << endl;
        return 1;
    }
    catch (CompilerError e) {
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
