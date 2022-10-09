#ifndef __SEMANTICS_SYMBOL_HH__
#define __SEMANTICS_SYMBOL_HH__

class Symbol {
public:
    enum Kind {
        variable,
        function,
        syscall_param,
    };

    Symbol(Kind kind, std::string name) : _kind(kind), _name(name){};

    inline Kind kind() {
        return _kind;
    }

    inline std::string name() {
        return _name;
    }
private:
    Kind _kind;
    std::string _name;
};

#endif // __SEMANTICS_SYMBOL_HH__
