#include <compile.hh>
#include <stdexcept>
#include <algorithm>
#include <queue>
#include <utility>

// Tipos auxiliares
struct SyscallRulesWithNumber {
    int nr;
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
    _filter->push_back(default_action);

    for (int i = order.size() - 1; i >= 0; i--) {
        auto [nr, rules] = order[i];
    }

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
