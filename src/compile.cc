#include <compile.hh>
#include <stdexcept>
#include <algorithm>
#include <queue>
#include <utility>
#include <stddef.h>
#include <sstream>
#include <errors.hh>

#include <linux/audit.h>

constexpr auto ARCH =
#if defined(__i386__)
    AUDIT_ARCH_I386;
#elif defined(__x86_64__)
    AUDIT_ARCH_X86_64;
#elif defined(__arm__)
    AUDIT_ARCH_ARM;
#elif defined(__aarch64__)
    AUDIT_ARCH_AARCH64;
#else
#error "Unsupported architecture"
#endif


// Tipos auxiliares
struct SyscallRulesWithNumber {
    uint nr;
    std::shared_ptr<semantics::SyscallRules> rules;
};

// Cabeçalhos auxiliares
std::shared_ptr<semantics::Policy> get_policy(semantics::AnalysisResult * ar, std::string & entry);
sock_filter get_seccomp_ret(syntax::Action * action);
std::vector<SyscallRulesWithNumber> get_resolution_order(std::shared_ptr<semantics::PolicyRules> rules);

// Declaração da função de compilação
CompileResult compile(semantics::AnalysisResult *ar, std::string entry) {
    try {
        return CompileResult(ar, entry);
    }
    catch (CompilerError& e) {
        throw e.build(ar->filename());
    }
}

CompileResult compile(syntax::Program *program, std::string entry) {
    return CompileResult(program, entry);
}

CompileResult compile(std::string filename, std::string entry) {
    return CompileResult(filename, entry);
}

// Implementação da geração do filtro
CompileResult::CompileResult(semantics::AnalysisResult * ar, std::string entry) {
    auto policy = get_policy(ar, entry);

    auto order = get_resolution_order(policy->rules());

    _filter = std::make_unique<FilterVector>();

    using kind = syntax::Expr::Kind;

    auto default_action = get_seccomp_ret(policy->default_action().get());

    // Vamos construir o filtro de trás pra frente para depois revertê-lo
    _filter->push_back(default_action);

    // Para isso, vamos precisar de um mapa de posições, de forma que seremos
    // capazes de encontrar a posição do início da validação de uma syscall
    // e poder fazer os jumps de forma adequada.
    std::unordered_map<uint, uint> position_map;

    auto get_position = [&position_map](uint nr) {
        auto it = position_map.find(nr);

        if (it == position_map.end()) {
            return 0u;
        }

        return it->second;
    };

    for (int i = order.size() - 1; i >= 0; i--) {
        auto [nr, rules] = order[i];

        auto current_length = _filter->size();

        // Match. No caso curr_nr == nr[i]
        // Caso nenhuma regra passe, o filtro vai para o default_action,
        // que está na posição 0.
        if (current_length - 1) {
            _filter->push_back(
                BPF_STMT(BPF_JMP | BPF_JA, (uint) current_length - 1)
            );
        }

        // Como estamos construindo o filtro de trás pra frente, precisamos
        // iterar sobre as regras na ordem inversa.
        for (auto it = rules->rbegin(); it != rules->rend(); it++) {
            auto [expr, action] = *it;

            // Se não foi especificada uma condição, é porque esta será tratada
            // como sempre verdadeira.
            if (expr == nullptr) {
                expr = std::make_shared<syntax::Constant>(true);
            }

            auto constant = expr->kind() == kind::constant
                ? std::static_pointer_cast<syntax::Constant>(expr)
                : nullptr;

            // Se a condição for uma constante verdadeira, podemos ignorar
            // todo o código das condições definidas anteriormente, uma vez que
            // elas nunca serão executadas.
            if (constant && constant->is_truthy()) {
                // Podemos ainda otimizar o filtro, já que sabemos que todas as
                // condições inseridas acima nunca serão executadas.
                while (_filter->size() > current_length) {
                    _filter->pop_back();
                }

                _filter->push_back(get_seccomp_ret(action.get()));

                continue;
            }

            // Se for uma constante falsa, podemos ignorar esta condição, pois
            // sua ação nunca será executada.
            if (constant && !constant->is_truthy()) {
                continue;
            }

            // Destino da expressão verdadeira
            _filter->push_back(get_seccomp_ret(action.get()));

            // Jump para a finalização ou para a próxima expressão
            _filter->push_back(
                // Se o acumulador for 0, pula para a próxima regra. Caso
                // contrário, pula para o destino da regra
                BPF_JUMP(BPF_JMP | BPF_JGT | BPF_K, 0, 0, 1)
            );

            std::unique_ptr<FilterVector> compiled_expr;

            try {
                compiled_expr = compile_expr(expr.get());
            }
            catch (CompilerError& e) {
                throw e.push(expr->begin(), "expression");
            }

            for (auto it = compiled_expr->rbegin(); it != compiled_expr->rend(); it++) {
                _filter->push_back(*it);
            }
        }

        auto left_son = 2 * i + 1;
        auto right_son = 2 * i + 2;

        auto left_son_pos = get_position(left_son);
        auto right_son_pos = get_position(right_son);

        // Último JUMP. No caso curr_nr < nr[i]. Saltamos em direção ao filho
        // esquerdo
        uint jump_size = _filter->size() - left_son_pos - 1;
        bool additional_instruction = jump_size > 0xff;

        // Se o jump for muito grande, precisamos inserir uma instrução adicional
        if (additional_instruction) {
            _filter->push_back(
                BPF_STMT(BPF_JMP | BPF_JA | BPF_K, jump_size)
            );
            jump_size = 0;
        }

        // Jump condicional. Aqui, já sabemos que curr_nr <= nr[i]. Se
        // curr_nr == nr[i], saltamos para a validação das regras. Caso
        // contrário, saltamos para a instrução de JUMP definida acima
        _filter->push_back(
            BPF_JUMP(
                BPF_JMP | BPF_JEQ | BPF_K, nr,
                additional_instruction, (__u8) jump_size
            )
        );

        // No caso curr_nr > nr[i]. Saltamos em direção ao filho direito
        jump_size = _filter->size() - right_son_pos - 1;
        additional_instruction = jump_size > 0xff;

        if (additional_instruction) {
            _filter->push_back(
                BPF_STMT(BPF_JMP | BPF_JA | BPF_K, jump_size)
            );
            jump_size = 0;
        }

        // Jump condicional. Aqui, vamos verificar se curr_nr > nr[i]. Se
        // for, saltamos para a instrução de JUMP definida acima. Caso
        // contrário, saltamos para a instrução de JUMP condicional acima, que
        // checará se curr_nr == nr[i]
        _filter->push_back(
            BPF_JUMP(
                BPF_JMP | BPF_JGT | BPF_K, nr,
                (__u8) jump_size, additional_instruction
            )
        );

        auto pos = _filter->size() - 1;

        position_map[i] = pos;
    }

    // Agora, vamos fazer o carregamento do número da syscall no acumulador
    // para a verificação pelos nossos filtros
    _filter->push_back(
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, SECCOMP_DATA(nr))
    );

    // Verificação da arquitetura

    // Primeiro, colocamos o destino final da arquitetura inválida:
    // o término do programa
    _filter->push_back(
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS)
    );

    // Agora, vamos verificar se a arquitetura é válida. Se for, vamos
    // pular para a instrução de carregamento do número da syscall. Caso
    // contrário, vamos pular para o destino final definido acima
    _filter->push_back(
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ARCH, 1, 0)
    );

    // Por fim (ou início), vamos carregar a arquitetura atual no acumulador
    _filter->push_back(
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, SECCOMP_DATA(arch))
    );

    std::reverse(_filter->begin(), _filter->end());
}

CompileResult::CompileResult(syntax::Program *program, std::string entry)
    : CompileResult(semantics::analyze(program).get(), entry) {}

CompileResult::CompileResult(std::string filename, std::string entry)
    : CompileResult(semantics::analyze(filename).get(), entry) {}

// Conversão do filtro para código em C
CompileResult::operator std::string() {
    std::stringstream ss;

    for (auto &[code, jt, jf, k]: *_filter) {
        auto op_class = code & 0x07;

        switch (op_class) {
            case BPF_JMP: {
                if ((code & 0xf0) == BPF_JA) {
                    ss << "BPF_STMT(";
                }
                else {
                    ss << "BPF_JUMP(";
                }

                ss << "BPF_JMP | ";

                switch (code & 0xf0) {
                    case BPF_JA:
                        ss << "BPF_JA  | ";
                        break;
                    case BPF_JEQ:
                        ss << "BPF_JEQ | ";
                        break;
                    case BPF_JGT:
                        ss << "BPF_JGT | ";
                        break;
                    case BPF_JGE:
                        ss << "BPF_JGE | ";
                        break;
                    case BPF_JSET:
                        ss << "BPF_JSET | ";
                        break;
                }

                break;
            }
            case BPF_LD: {
                ss << "BPF_STMT(BPF_LD  | ";

                switch(code & 0x18) {
                    case BPF_W:
                        ss << "BPF_W   | ";
                        break;
                    case BPF_H:
                        ss << "BPF_H   | ";
                        break;
                    case BPF_B:
                        ss << "BPF_B   | ";
                        break;
                }

                switch (code & 0xe0) {
                    case BPF_ABS:
                        ss << "BPF_ABS, ";
                        break;
                    case BPF_IND:
                        ss << "BPF_IND, ";
                        break;
                    case BPF_MEM:
                        ss << "BPF_MEM, ";
                        break;
                    case BPF_LEN:
                        ss << "BPF_LEN, ";
                        break;
                    case BPF_IMM:
                        ss << "BPF_IMM, ";
                        break;
                }

                break;
            }
            case BPF_LDX: {
                ss << "BPF_STMT(BPF_LDX | ";

                switch(code & 0x18) {
                    case BPF_W:
                        ss << "BPF_W   | ";
                        break;
                    case BPF_H:
                        ss << "BPF_H   | ";
                        break;
                    case BPF_B:
                        ss << "BPF_B   | ";
                        break;
                }

                switch (code & 0xe0) {
                    case BPF_IMM:
                        ss << "BPF_IMM, ";
                        break;
                    case BPF_MEM:
                        ss << "BPF_MEM, ";
                        break;
                    case BPF_LEN:
                        ss << "BPF_LEN, ";
                        break;
                    case BPF_MSH:
                        ss << "BPF_MSH, ";
                        break;
                }

                break;
            }
            case BPF_RET: {
                ss << "BPF_STMT(BPF_RET | ";
                break;
            }
            case BPF_ALU: {
                ss << "BPF_STMT(BPF_ALU | ";

                switch(code & 0xf0) {
                    case BPF_ADD:
                        ss << "BPF_ADD | ";
                        break;
                    case BPF_SUB:
                        ss << "BPF_SUB | ";
                        break;
                    case BPF_MUL:
                        ss << "BPF_MUL | ";
                        break;
                    case BPF_DIV:
                        ss << "BPF_DIV | ";
                        break;
                    case BPF_OR:
                        ss << "BPF_OR  | ";
                        break;
                    case BPF_AND:
                        ss << "BPF_AND | ";
                        break;
                    case BPF_LSH:
                        ss << "BPF_LSH | ";
                        break;
                    case BPF_RSH:
                        ss << "BPF_RSH | ";
                        break;
                    case BPF_NEG:
                        ss << "BPF_NEG | ";
                        break;
                    case BPF_MOD:
                        ss << "BPF_MOD | ";
                        break;
                    case BPF_XOR:
                        ss << "BPF_XOR | ";
                        break;
                }

                break;
            }
            case BPF_ST: {
                ss << "BPF_STMT(BPF_ST  | ";

                break;
            }
            case BPF_MISC: {
                ss << "BPF_STMT(BPF_MISC| ";

                switch(code & 0xf0) {
                    case BPF_TAX:
                        ss << "BPF_TAX | ";
                        break;
                    case BPF_TXA:
                        ss << "BPF_TXA | ";
                        break;
                }

                break;
            }
            case BPF_STX: {
                ss << "BPF_STMT(BPF_STX | ";
                break;
            }
        }

        if (op_class != BPF_LD && op_class != BPF_LDX) {
            switch (code & 0x08) {
                case BPF_K:
                    ss << "BPF_K, ";
                    break;
                case BPF_X:
                    ss << "BPF_X, ";
                    break;
            }
        }

        if ((code & 0x07) == BPF_RET) {
            switch (k & 0xffff0000) {
                case SECCOMP_RET_ALLOW:
                    ss << "SECCOMP_RET_ALLOW";
                    break;
                case SECCOMP_RET_KILL_THREAD:
                    ss << "SECCOMP_RET_KILL";
                    break;
                case SECCOMP_RET_ERRNO:
                    ss << "SECCOMP_RET_ERRNO | ("
                        << (k & 0x0000ffff) << " & SECCOMP_RET_DATA)";
                    break;
                case SECCOMP_RET_TRACE:
                    ss << "SECCOMP_RET_TRACE | ("
                        << (k & 0x0000ffff) << " & SECCOMP_RET_DATA)";
                    break;
                case SECCOMP_RET_LOG:
                    ss << "SECCOMP_RET_LOG";
                    break;
                case SECCOMP_RET_TRAP:
                    ss << "SECCOMP_RET_TRAP | ("
                        << (k & 0x0000ffff) << " & SECCOMP_RET_DATA)";
                    break;
                case SECCOMP_RET_USER_NOTIF:
                    ss << "SECCOMP_RET_USER_NOTIF";
                    break;
                case SECCOMP_RET_KILL_PROCESS:
                    ss << "SECCOMP_RET_KILL_PROCESS";
                    break;
            }
        }
        else {
            ss << k;
        }

        if ((code & 0x07) == BPF_JMP && (code & 0xf0) != BPF_JA) {
            ss << ", " << (uint) jt << ", " << (uint) jf;
        }

        ss << "),\n";
    }

    return ss.str();
}

// Conversão do filtro para estrutura sock_fprog
CompileResult::operator sock_fprog() {
    return sock_fprog {
        .len = (unsigned short) _filter->size(),
        .filter = _filter->data()
    };
}

std::shared_ptr<semantics::Policy> get_policy(semantics::AnalysisResult *ar, std::string &entry) {
    auto it = ar->policies()->find(entry);

    if (it == ar->policies()->end()) {
        throw CompilerError("Policy not found: " + entry);
    }

    return it->second;
}

sock_filter get_seccomp_ret(syntax::Action *action) {
    static const ushort CODE = BPF_RET | BPF_K;

    using syntax::Action;

    switch (action->action_kind()) {
        case Action::Kind::error:
            return BPF_STMT(CODE, SECCOMP_RET_ERRNO | SECCOMP_RET_DATA & action->param());
        case Action::Kind::trap:
            return BPF_STMT(CODE, SECCOMP_RET_TRAP | SECCOMP_RET_DATA & action->param());
        case Action::Kind::trace:
            return BPF_STMT(CODE, SECCOMP_RET_TRACE | SECCOMP_RET_DATA & action->param());
        case Action::Kind::allow:
            return BPF_STMT(CODE, SECCOMP_RET_ALLOW);
        case Action::Kind::kill:
            return BPF_STMT(CODE, SECCOMP_RET_KILL);
        case Action::Kind::notify:
            return BPF_STMT(CODE, SECCOMP_RET_USER_NOTIF);
        case Action::Kind::log:
            return BPF_STMT(CODE, SECCOMP_RET_LOG);
        case Action::Kind::terminate:
            return BPF_STMT(CODE, SECCOMP_RET_KILL_PROCESS);
        default:
            throw std::runtime_error("Never should be here");
    }
}

std::vector<SyscallRulesWithNumber> get_resolution_order(std::shared_ptr<semantics::PolicyRules> rules) {
    static auto log = [](int x) {
        int ret = 0;

        while (x >>= 1) ++ret;

        return ret;
    };

    using Range = std::pair<int, int>;

    std::queue<Range> queue;

    auto n = rules->size();

    std::vector<SyscallRulesWithNumber> rules_list;
    std::vector<SyscallRulesWithNumber> result;

    rules_list.reserve(n);
    result.reserve(n);

    for (auto [nr, sc_rules] : *rules) {
        rules_list.push_back({nr, sc_rules});
    }

    queue.push({0, n});

    while (!queue.empty()) {
        const auto [i, j] = queue.front();
        queue.pop();

        const auto size = j - i;

        const auto k = 1 << log(size + 1);

        const auto s = size - k + 1;
        const auto sr = std::max(s - k / 2, 0);
        const auto sl = s - sr;

        const auto m = i + (sl + size - 1 - sr) / 2;

        result.push_back(rules_list[m]);

        if (i < m) {
            queue.push({i, m});
        }

        if (m + 1 < j) {
            queue.push({m + 1, j});
        }
    }

    return result;
}
