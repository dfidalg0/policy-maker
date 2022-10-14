#ifndef __SYNTAX_NODES_CONSTANT_HH__
#define __SYNTAX_NODES_CONSTANT_HH__

#include "_expr.hh"

namespace syntax {
    class Constant : public syntax::Expr {
    public:
        enum class Type {
            integer,
            string,
            boolean,
            null,
        };

        static std::string type_to_string(Type type);

        bool is_truthy();

    private:
        std::string _value;
        Type _type;

    public:
        Constant(
            Type type,
            std::string value,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : syntax::Expr(Node::Kind::constant, begin, end), _type(type), _value(value) {}

        Constant(
            char const * value,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Constant(Type::string, value, begin, end) {}

        Constant(
            std::string value,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Constant(Type::string, value, begin, end) {}

        Constant(
            int value,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Constant(Type::integer, std::to_string(value), begin, end) {}

        Constant(
            bool value,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Constant(Type::boolean, value ? "true" : "false", begin, end) {}

        Constant(
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ) : Constant(Type::null, "null", begin, end) {}

        ~Constant() {}

        inline std::string value() const { return _value; }
        inline Type type() const { return _type; }

        void print(uint level = 0) override;

        operator int();
        operator std::string();
        operator bool();
    };
};

#endif // __SYNTAX_NODES_CONSTANT_HH__
