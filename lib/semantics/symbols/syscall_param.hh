#ifndef __SEMANTICS_SYMBOLS_SYSCALL_PARAM_HH__
#define __SEMANTICS_SYMBOLS_SYSCALL_PARAM_HH__

#include <string>

#include <semantics/symbols/_symbol.hh>

namespace semantics {
    class SyscallParam : public Symbol {
    public:
        SyscallParam(std::string name, int index, bool pointer);
        virtual ~SyscallParam(){};

        inline int index() const { return _index; };

        inline bool pointer() const { return _pointer; };
    private:
        int _index;
        bool _pointer;
    };
};


#endif // __SEMANTICS_SYMBOLS_SYSCALL_PARAM_HH__
