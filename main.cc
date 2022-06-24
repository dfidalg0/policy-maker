#include <iostream>
#include <parser.yy.hh>

using std::cout;
using std::endl;

int main(int argc, char * argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <file>" << endl;
        return 1;
    }

    auto program = parse(argv[1]);

    if (!program) {
        cout << "Error: file not found" << endl;
        return 1;
    }

    program->print();

    return 0;
}
