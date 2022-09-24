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

        program->print();

        compile(program);
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
