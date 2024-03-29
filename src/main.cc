#include <iostream>
#include <semantics/main.hh>
#include <compile.hh>
#include <utils/cmd_parse.hh>
#include <unistd.h>
#include <cstring>

#include <run_seccomp.hh>
#include <parser.yy.hh>
#include <write_compiled.hh>

using std::cout;
using std::endl;
using std::cerr;

int main(int argc, char const * argv[]) {
    const auto cmd_parser = utils::CmdParser()
        .add_help_arg()
        .set_rest_description(
            "If present, any arguments after the compiler options will be passed to execvp after the filter is installed. No output will be generated."
        )
        .add_arg({
            .name = "input",
            .type = utils::ArgType::STRING,
            .required = true,
            .positional = true,
            .description = "The input file to compile",
            .shorthand = "i",
        })
        .add_arg({
            .name = "output",
            .type = utils::ArgType::STRING,
            .required = false,
            .positional = false,
            .description = "The output file to write to",
            .shorthand = "o",
            .default_value = "filter.c",
        })
        .add_arg({
            .name = "dry-run",
            .type = utils::ArgType::NONE,
            .required = false,
            .positional = false,
            .description = "Don't produce any output. Just validates the input file",
            .shorthand = "d",
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
        })
        .add_arg({
            .name = "target",
            .type = utils::ArgType::STRING,
            .required = false,
            .positional = false,
            .description = "Name of the generated function",
            .shorthand = "t",
            .default_value = "install_filter"
        });

    try {
        auto options = cmd_parser.parse(argc, argv);

        auto ast = syntax::parse(options.get("input")).release();

        if (options.has("print-ast")) {
            cout << "AST:\n";
            ast->print();
            cout << '\n';
        }

        auto result = semantics::analyze(ast).release();

        delete ast;

        if (options.has("print-analyzed-ast")) {
            cout << "Analyzed AST:\n";
            result->print();
            cout << '\n';
        }

        auto compile_result = compile(result, options.get("entry"));

        delete result;

        sock_fprog prog = compile_result;

        if (options.has("print-bpf")) {
            std::string prog_str = compile_result;

            cout << "Compiled program:\n";

            cout << prog_str << endl;
        }

        auto args = options.rest();

        if (args.size()) {
            return run_seccomp(prog, args);
        }

        if (options.has("output") && !options.has("dry-run")) {
            write_compiled(
                options.get("output"),
                options.get("target"),
                compile_result
            );
        }

        cout << "Code compiled successfully" << endl;
        return 0;
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
