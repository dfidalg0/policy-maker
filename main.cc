#include <iostream>
#include <parser.yy.hh>

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
    }
    catch (FileNotFoundError e) {
        cerr << e.what() << endl;
    }
    catch(ParseError e) {
        cerr << e.what() << endl;
    }
    catch (...) {
        cerr << "Unknown error" << endl;
    }

    return 0;
}
