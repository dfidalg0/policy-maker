#ifndef __COMPILE_EXPR_HH__
#define __COMPILE_EXPR_HH__

#include <vector>
#include <memory>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <syntax/nodes.hh>
#include <errors.hh>

#define SECCOMP_DATA(name) offsetof(seccomp_data, name)

typedef std::vector<sock_filter> FilterVector;

std::unique_ptr<FilterVector> compile_expr(syntax::Expr * expr);

class RegisterPool {
public:
    class Register {
    private:
        Register(uint reg) : _nr(reg), _is_used(false) {}

        bool _is_used;
        uint _nr;

        friend class RegisterPool;

    public:
        void release() {
            _is_used = false;
        }

        inline uint nr() {
            return _nr;
        }
    };

    RegisterPool() {
        for (uint i = 0; i < BPF_MEMWORDS; ++i) {
            _registers.push_back(Register(i));
        }
    }

    Register& get() {
        for (auto& reg : _registers) {
            if (!reg._is_used) {
                reg._is_used = true;
                return reg;
            }
        }

        throw CompilerError("Expression too complex");
    }

private:
    std::vector<Register> _registers;
};

#endif // __COMPILE_EXPR_HH__
