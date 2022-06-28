#ifndef __ACTION_HH__
#define __ACTION_HH__

#include <token.hh>
#include <nodes/_node.hh>
#include <exception>

class Action : public Node {
public:
    enum class Kind {
        allow,
        kill,
        trap,
        notify,
        trace,
        log,
        terminate,
    };

    static std::string kind_to_string(Kind kind);

    class Exception : public std::exception {
    public:
        Exception(std::string message) : _message(message) {}
        virtual const char* what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message;
    };

    static Kind kind_from_token(Token * token);

    Action(Kind kind, Position begin, Position end, int param = -1)
        : Node(Node::Kind::action, begin, end), _action_kind(kind), _param(param) {}

    inline Kind action_kind() const { return _action_kind; }
    inline int param() const { return _param; }

    void print(uint level = 0) override;

private:
    Kind _action_kind;
    int _param;
};

#endif // __ACTION_HH__
