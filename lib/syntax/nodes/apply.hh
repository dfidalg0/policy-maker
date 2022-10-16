#ifndef __SYNTAX_NODES_APPLY_HH__
#define __SYNTAX_NODES_APPLY_HH__

#include "_stmt.hh"
#include <string>

namespace syntax {
    class Apply: public Stmt {
    public:
        Apply(
            std::string policy,
            Position begin = Position(0, 0),
            Position end = Position(0, 0)
        ): Stmt(Kind::apply, begin, end), _policy(policy) {}

        inline std::string policy() { return _policy; }

        void print(uint level = 0) override;
    private:
        std::string _policy;
    };
};

#endif // __SYNTAX_NODES_APPLY_HH__
