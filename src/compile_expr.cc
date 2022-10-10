#include <compile_expr.hh>

BinaryExpr::OpKind get_oposite(BinaryExpr::OpKind op);

FilterVector * compile_expr(Expr * expr) {
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
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
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
            auto op = binary->op();

            using Op = BinaryExpr::OpKind;

            auto * Lconst = L->kind() == kind::constant
                ? (Constant *) L
                : nullptr;

            auto * Rconst = R->kind() == kind::constant
                ? (Constant *) R
                : nullptr;

            // Curto circuito para o operador lógico OR
            if (op == Op::lor) {
                if (
                    (Lconst && Lconst->is_truthy()) ||
                    (Rconst && Rconst->is_truthy())
                ) {
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    break;
                }

                // Nunca acontecerá o caso com duas constantes, pois isso já
                // foi otimizado anteriormente

                // L é uma constante falsa
                if (Lconst) {
                    auto Rfilter = compile_expr(R);
                    delete filter;
                    filter = Rfilter;
                    break;
                }

                // R é uma constante falsa
                if (Rconst) {
                    auto Lfilter = compile_expr(L);
                    delete filter;
                    filter = Lfilter;
                    break;
                }

                auto Rfilter = compile_expr(R);

                auto Lfilter = compile_expr(L);
                filter->insert(filter->end(), Lfilter->begin(), Lfilter->end());
                delete Lfilter;

                __u8 rsize = Rfilter->size();

                filter->push_back(
                    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0, 0, rsize)
                );

                filter->insert(filter->end(), Rfilter->begin(), Rfilter->end());
                delete Rfilter;

                break;
            }

            // Curto circuito para o operador lógico AND
            if (op == Op::land) {
                if (
                    (Lconst && !Lconst->is_truthy()) ||
                    (Rconst && !Rconst->is_truthy())
                ) {
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
                    );
                    break;
                }

                // Nunca acontecerá o caso com duas constantes, pois isso já
                // foi otimizado anteriormente

                // L é uma constante verdadeira
                if (Lconst) {
                    auto Rfilter = compile_expr(R);
                    delete filter;
                    filter = Rfilter;
                    break;
                }

                // R é uma constante verdadeira
                if (Rconst) {
                    auto Lfilter = compile_expr(L);
                    delete filter;
                    filter = Lfilter;
                    break;
                }

                auto Rfilter = compile_expr(R);

                auto Lfilter = compile_expr(L);
                filter->insert(filter->end(), Lfilter->begin(), Lfilter->end());
                delete Lfilter;

                __u8 rsize = Rfilter->size();

                filter->push_back(
                    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0, rsize, 0)
                );

                filter->insert(filter->end(), Rfilter->begin(), Rfilter->end());
                delete Rfilter;

                break;
            }

            uint source_reg = BPF_X;
            uint k = 0;

            if (Lconst) {
                switch (op) {
                    case Op::gt:
                    case Op::lt:
                    case Op::ge:
                    case Op::le: {
                        op = get_oposite(op);
                    }
                    case Op::add:
                    case Op::mul:
                    case Op::bit_and:
                    case Op::bit_or:
                    case Op::bit_xor:
                    case Op::eq:
                    case Op::ne: {
                        std::swap(R, L);
                        std::swap(Lconst, Rconst);
                        break;
                    }
                    default: {
                        int left_value = *Lconst;

                        auto r_code = compile_expr(R);
                        filter->insert(filter->end(), r_code->begin(), r_code->end());
                        delete r_code;

                        // Carregamos o acumulador no registro de índice
                        filter->push_back(
                            BPF_STMT(BPF_MISC | BPF_TAX, 0)
                        );

                        // Carregamos o valor da constante no acumulador
                        filter->push_back(
                            BPF_STMT(BPF_LD | BPF_IMM, (uint)left_value)
                        );
                    }
                }
            }

            if (Rconst) {
                int right_value = *Rconst;

                auto l_code = compile_expr(L);
                filter->insert(filter->end(), l_code->begin(), l_code->end());
                delete l_code;

                source_reg = BPF_K;
                k = right_value;
            }

            if (!Lconst && !Rconst) {
                // Carregamos a expressão direita no acumulador
                auto r_code = compile_expr(R);
                filter->insert(filter->end(), r_code->begin(), r_code->end());
                delete r_code;

                // Obtemos um espaço livre na memória
                auto r_reg = pool.get();

                // Salvamos o resultado da expressão direita na memória que
                // acabamos de obter
                filter->push_back(
                    BPF_STMT(BPF_ST, r_reg.nr())
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
            switch (op) {
                case BinaryExpr::OpKind::add: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_ADD | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::sub: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_SUB | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::mul: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_MUL | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::div: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_DIV | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::mod: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_MOD | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::bit_and: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_AND | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::bit_or: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_OR | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::bit_xor: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_XOR | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::bit_lsh: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_LSH | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::bit_rsh: {
                    filter->push_back(BPF_STMT(BPF_ALU | BPF_RSH | source_reg, k));
                    break;
                }
                case BinaryExpr::OpKind::eq: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JEQ | source_reg, k, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::ne: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JEQ | source_reg, k, 2, 0)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::gt: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGT | source_reg, k, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::lt: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGE | source_reg, k, 2, 0)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::ge: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGE | source_reg, k, 0, 2)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
                    );
                    break;
                }
                case BinaryExpr::OpKind::le: {
                    filter->push_back(
                        BPF_JUMP(BPF_JMP | BPF_JGT | source_reg, k, 2, 0)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_JMP | BPF_JA | BPF_K, 1)
                    );
                    filter->push_back(
                        BPF_STMT(BPF_LD | BPF_IMM, 0)
                    );
                    break;
                }
            }

            break;
        }
    }

    return filter;
}

BinaryExpr::OpKind get_oposite(BinaryExpr::OpKind op) {
    using Op = BinaryExpr::OpKind;

    switch (op) {
        case Op::gt: return Op::le;
        case Op::lt: return Op::ge;
        case Op::ge: return Op::lt;
        case Op::le: return Op::gt;
        default: throw std::runtime_error("Invalid op");
    }
}
