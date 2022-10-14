#ifndef __LEXICON_POSITION_HH__
#define __LEXICON_POSITION_HH__

class Position {
private:
    unsigned _line;
    unsigned _col;
public:
    Position(unsigned line, unsigned col) : _line(line), _col(col) {}
    inline unsigned line() const { return _line; }
    inline unsigned col() const { return _col; }
};

#endif // __LEXICON_POSITION_HH__
