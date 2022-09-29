#ifndef __CONSTANT_HH__
#define __CONSTANT_HH__

#include "_expr.hh"

class Constant : public Expr {
public:
    enum class Type {
        integer,
        string,
        boolean,
        null,
    };

    static std::string type_to_string(Type type);

private:
    std::string _value;
    Type _type;

public:
    Constant(Type type, std::string value, Position begin, Position end)
        : Expr(Node::Kind::constant, begin, end), _type(type), _value(value) {}

    Constant(char const * value, Position begin, Position end)
        : Constant(Type::string, value, begin, end) {}

    Constant(std::string value, Position begin, Position end)
        : Constant(Type::string, value, begin, end) {}

    Constant(int value, Position begin, Position end)
        : Constant(Type::integer, std::to_string(value), begin, end) {}

    Constant(bool value, Position begin, Position end)
        : Constant(Type::boolean, value ? "true" : "false", begin, end) {}

    Constant(Position begin, Position end)
        : Constant(Type::null, "null", begin, end) {}

    ~Constant() {}

    inline std::string value() const { return _value; }
    inline Type type() const { return _type; }

    void print(uint level = 0) override;

    operator int();
    operator std::string();
    operator bool();
};

#endif // __CONSTANT_HH__
