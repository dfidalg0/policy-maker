#include <compile_expr.hh>

class RegisterPool {
public:
    class Register {
    private:
        Register(uint reg) {
            _nr = reg;
        }

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
        for (auto &reg: _registers) {
            if (!reg._is_used) {
                reg._is_used = true;
                return reg;
            }
        }

        throw std::runtime_error("Expression too complex");
    }
private:
    std::vector<Register> _registers;
};

std::vector<sock_filter> * compile_expr(Expr * expr) {
    static auto pool = RegisterPool();

    auto filter = new std::vector<struct sock_filter>();

    using kind = Expr::Kind;

    switch (expr->kind()) {
        case kind::syscall_param: {
            auto param = (SyscallParam *) expr;

            auto idx = param->index();

            auto offset = (__u32) (SECCOMP_DATA(args[0]) + idx * sizeof(__u64));

            filter->push_back(
                BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offset)
            );

            break;
        }
        case kind::unary_expr: {
            auto unary = (UnaryExpr *) expr;

            auto operand = compile_expr(unary->operand());

            filter->insert(filter->end(), operand->begin(), operand->end());

            delete operand;

            switch (unary->op()) {
                case UnaryExpr::OpKind::neg: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_NEG, 0));
                    break;
                }
                case UnaryExpr::OpKind::pos: break;
                // Bitwise not
                case UnaryExpr::OpKind::bit_not: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_NEG, 0));
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_SUB | BPF_K, 1));
                    break;
                }
                // Logical not
                case UnaryExpr::OpKind::lnot: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
            }

            break;
        }
        case kind::binary_expr: {
            auto binary = (BinaryExpr *) expr;

            auto L = binary->left();
            auto R = binary->right();

            if (L->kind() == kind::constant) {
                int left_value = *((Constant *) L);

                auto r_code = compile_expr(R);
                filter->insert(filter->end(), r_code->begin(), r_code->end());
                delete r_code;

                // Carregamos o acumulador no registro de índice
                filter->push_back(
                    BPF_STMT(BPF_MISC | BPF_TAX, 0)
                );

                // Carregamos o valor da constante no acumulador
                filter->push_back(
                    BPF_STMT(BPF_LD | BPF_IMM, (uint) left_value)
                );
            }
            else if (R->kind() == kind::constant) {
                int right_value = *((Constant *) R);

                auto l_code = compile_expr(L);
                filter->insert(filter->end(), l_code->begin(), l_code->end());
                delete l_code;

                // Carregamos o valor da constante no registro de índice
                filter->push_back(
                    BPF_STMT(BPF_LDX | BPF_W | BPF_IMM, (uint) right_value)
                );
            }
            else {
                // Carregamos a expressão direita no acumulador
                auto r_code = compile_expr(R);
                filter->insert(filter->end(), r_code->begin(), r_code->end());
                delete r_code;

                // Obtemos um espaço livre na memória
                auto r_reg = pool.get();

                // Salvamos o resultado da expressão direita na memória que
                // acabamos de obter
                filter->push_back(
                    BPF_STMT(BPF_ST | BPF_W | BPF_MEM, r_reg.nr())
                );

                // Carregamos a expressão esquerda no acumulador
                auto l_code = compile_expr(L);
                filter->insert(filter->end(), l_code->begin(), l_code->end());
                delete l_code;

                // Carregamos o valor da memória no registro de índice
                filter->push_back(
                    BPF_STMT(BPF_LDX | BPF_W | BPF_MEM, r_reg.nr())
                );

                // Liberamos a memória que acabamos de usar
                r_reg.release();
            }

            // Realizamos a operação
            switch (binary->op()) {
                case BinaryExpr::OpKind::add: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_ADD | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::sub: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_SUB | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::mul: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_MUL | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::div: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_DIV | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::mod: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_MOD | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::bit_and: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_AND | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::bit_or: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_OR | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::bit_xor: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_XOR | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::bit_lsh: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_LSH | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::bit_rsh: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_RSH | BPF_X, 0));
                    break;
                }
                case BinaryExpr::OpKind::eq: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X, 0, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::ne: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X, 0, 2, 0)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::gt: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGT | BPF_X, 0, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::lt: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGE | BPF_X, 0, 2, 0)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::ge: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGE | BPF_X, 0, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::le: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGT | BPF_X, 0, 2, 0)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::land: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JSET | BPF_X, 0, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::lor: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_OR | BPF_X, 0));
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0, 2, 0)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_W | BPF_IMM, 0)
                    );
                    break;
                }
            }

            break;
        }
    }

    return filter;
}
