#include <lexicon/position.hh>

std::ostream& operator<<(std::ostream& os, const Position& pos) {
    os << pos.line() << ":" << pos.col();
    return os;
}
