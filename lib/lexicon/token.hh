#ifndef __LEXICON_TOKEN_HH__
#define __LEXICON_TOKEN_HH__

#include <lexicon/position.hh>
#include <token_types.hh>
#include <string>

class Token {
private:
    Position _begin;
    Position _end;
    std::string _text;
    yytokentype _type;
public:
    Token(yytokentype type, Position begin, Position end, std::string text) : _begin(begin), _end(end), _text(text), _type(type) {}

    inline Position begin() const { return _begin; }
    inline Position end() const { return _end; }
    inline std::string text() const { return _text; }
    inline yytokentype type() const { return _type; }
};

#endif // __LEXICON_TOKEN_HH__
