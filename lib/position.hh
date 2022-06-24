#ifndef __POSITION_HH__
#define __POSITION_HH__

class Position {
private:
    unsigned _line;
    unsigned _col;
public:
    Position(unsigned line, unsigned col) : _line(line), _col(col) {}
    inline unsigned line() const { return _line; }
    inline unsigned col() const { return _col; }
};

#endif // __POSITION_HH__
