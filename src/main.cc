#include <iostream>
#include <syntax/main.hh>
#include <semantics/main.hh>

using std::cout;
using std::endl;
using std::cerr;

int main(int argc, char * argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <file>" << endl;
        return 1;
    }

    try {
        auto program = parse(argv[1]);

        auto policies = compile(program);

        for (auto & policy : *policies) {
            cout << "Policy: " << policy.first << endl;

            for (auto & syscall : *policy.second) {
                cout << "  Syscall: " << syscall.first << endl;

                for (auto & rule : *syscall.second) {
                    cout
                        << "    Action: "
                        << Action::kind_to_string(rule.second->action_kind());

                    if (rule.second->param() != -1) {
                        cout << "(" << rule.second->param() << ")";
                    }

                    if (rule.first) {
                        std::cout << " if\n";
                        rule.first->print(6);
                    }
                    else {
                        std::cout << endl;
                    }
                }
            }
        }
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
