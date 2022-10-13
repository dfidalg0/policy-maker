#ifndef __SYNTAX_NODES_ACTION_HH__
#define __SYNTAX_NODES_ACTION_HH__

#include <exception>
#include <lexicon/token.hh>

#include "_node.hh"

class Action : public Node {
public:
    enum class Kind {
        error,
        trap,
        trace,
        allow,
        kill,
        notify,
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

    Action(
        Kind kind,
        int param = -1,
        Position begin = Position(0, 0),
        Position end = Position(0, 0)
    )
        : Node(Node::Kind::action, begin, end), _action_kind(kind), _param(param) {}

    inline Kind action_kind() const { return _action_kind; }
    inline int param() const { return _param; }

    void print(uint level = 0) override;

private:
    Kind _action_kind;
    int _param;
};

#endif // __SYNTAX_NODES_ACTION_HH__
