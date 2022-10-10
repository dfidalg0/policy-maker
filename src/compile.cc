#include <compile.hh>
#include <stdexcept>
#include <algorithm>
#include <queue>
#include <utility>
#include <stddef.h>

#define SECCOMP_DATA(name) offsetof(struct seccomp_data, name)

// Tipos auxiliares
struct SyscallRulesWithNumber {
    uint nr;
    SyscallRules *rules;
};

// Cabeçalhos auxiliares
AnalysisResultPolicy * get_policy(AnalysisResult * ar, std::string & target);
sock_filter get_seccomp_ret(Action * action);
std::vector<SyscallRulesWithNumber> get_resolution_order(PolicyRules *rules);

// Declaração da função de compilação
CompileResult compile(AnalysisResult *ar, std::string target) {
    return CompileResult(ar, target);
}

CompileResult compile(Program *program, std::string target) {
    return CompileResult(program, target);
}

CompileResult compile(std::string filename, std::string target) {
    return CompileResult(filename, target);
}

// Implementação da geração do filtro
CompileResult::CompileResult(AnalysisResult * ar, std::string target) {
    auto policy = get_policy(ar, target);

    auto order = get_resolution_order(policy->rules());

    _filter = new std::vector<sock_filter>();

    using kind = Expr::Kind;

    auto default_action = get_seccomp_ret(policy->default_action());

    // Vamos construir o filtro de trás pra frente para depois revertê-lo

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
        // Caso nenhuma regra passe, o filtro vai para o default_action
        _filter->push_back(default_action);

        // Como estamos construindo o filtro de trás pra frente, precisamos
        // iterar sobre as regras na ordem inversa.
        for (auto it = rules->rbegin(); it != rules->rend(); it++) {
            auto [expr, action] = *it;

            // Se não foi especificada uma condição, é porque esta será tratada
            // como sempre verdadeira.
            if (expr == nullptr) {
                expr = new Constant(true, { 0, 0 }, { 0, 0 });
            }

            auto constant = expr->kind() == kind::constant
                ? (Constant *) expr
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

                _filter->push_back(get_seccomp_ret(action));

                continue;
            }

            // Se for uma constante falsa, podemos ignorar esta condição, pois
            // sua ação nunca será executada.
            if (constant && !constant->is_truthy()) {
                continue;
            }

            // Destino da expressão verdadeira
            _filter->push_back(get_seccomp_ret(action));

            // Jump para a finalização ou para a próxima expressão
            _filter->push_back(
                // Se o acumulador for 0, pula para a próxima regra. Caso
                // contrário, pula para o destino da regra
                BPF_JUMP(BPF_JMP | BPF_JGE | BPF_K, 0, 0, 1)
            );

            // TODO: Compilar a expressão para o filtro.
            // Por ora, vamos apenas carregar 1 no acumulador, assumindo que
            // a expressão é sempre verdadeira
            _filter->push_back(BPF_STMT(BPF_LD | BPF_IMM, 1));
        }

        auto left_son = 2 * i + 1;
        auto right_son = 2 * i + 2;

        auto left_son_pos = get_position(left_son);
        auto right_son_pos = get_position(right_son);

        // Último JUMP. No caso curr_nr < nr[i]. Saltamos em direção ao filho
        // esquerdo
        if (left_son_pos == 0) {
            _filter->push_back(default_action);
        }
        else {
            _filter->push_back(
                BPF_STMT(
                    BPF_JMP | BPF_JA | BPF_K,
                    (uint) _filter->size() - left_son_pos - 1
                )
            );
        }


        // Jump condicional. Aqui, já sabemos que curr_nr <= nr[i]. Se
        // curr_nr == nr[i], saltamos para a validação das regras. Caso
        // contrário, saltamos para a instrução de JUMP definida acima
        _filter->push_back(
            BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, nr, 1, 0)
        );

        // No caso curr_nr > nr[i]. Saltamos em direção ao filho direito
        if (right_son_pos == 0) {
            _filter->push_back(default_action);
        }
        else {
            _filter->push_back(
                BPF_STMT(
                    BPF_JMP | BPF_JA | BPF_K,
                    (uint) _filter->size() - right_son_pos - 1
                )
            );
        }

        // Jump condicional. Aqui, vamos verificar se curr_nr > nr[i]. Se
        // for, saltamos para a instrução de JUMP definida acima. Caso
        // contrário, saltamos para a instrução de JUMP condicional acima, que
        // checará se curr_nr == nr[i]
        _filter->push_back(
            BPF_JUMP(BPF_JMP | BPF_JGT | BPF_K, nr, 0, 1)
        );

        auto pos = _filter->size() - 1;

        position_map[i] = pos;
    }

    // Por fim (ou início), vamos fazer o carregamento do número da syscall
    // no acumulador
    _filter->push_back(
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, SECCOMP_DATA(nr))
    );

    std::reverse(_filter->begin(), _filter->end());
}

CompileResult::CompileResult(Program *program, std::string target)
    : CompileResult(analyze(program), target) {}

CompileResult::CompileResult(std::string filename, std::string target)
    : CompileResult(analyze(filename), target) {}

// Conversão do filtro para código em C
CompileResult::operator std::string() {
    throw std::runtime_error("Not implemented");
}

// Conversão do filtro para estrutura sock_fprog
CompileResult::operator sock_fprog() {
    return sock_fprog {
        .len = (unsigned short) _filter->size(),
        .filter = _filter->data()
    };
}

AnalysisResultPolicy *get_policy(AnalysisResult *ar, std::string &target) {
    auto it = ar->policies()->find(target);

    if (it == ar->policies()->end()) {
        throw std::runtime_error("Policy not found");
    }

    return it->second;
}

sock_filter get_seccomp_ret(Action *action) {
    static const ushort CODE = BPF_RET | BPF_K;

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
            throw std::runtime_error("Invalid action");
    }
}

std::vector<SyscallRulesWithNumber> get_resolution_order(PolicyRules *rules) {
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

    for (auto [nr, sc_rules] : *rules) {
        rules_list.push_back({nr, sc_rules});
    }

    result.reserve(n);

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
