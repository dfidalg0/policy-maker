#include <run_seccomp.hh>

static char * const * get_argv(std::vector<std::string> &args) {
    auto size = args.size();

    const char **argv = new const char *[size + 1];

    for (uint i = 0; i < size; ++i) {
        argv[i] = args[i].c_str();
    }

    argv[size] = nullptr;

    return (char * const *)argv;
}

int run_seccomp(sock_fprog &prog, std::vector<std::string> &args) {
    int flags = 0;

    for (int i = 0; i < prog.len; ++i) {
        auto &filter = prog.filter[i];

        if (BPF_CLASS(filter.code) == BPF_RET && filter.k == SECCOMP_RET_USER_NOTIF) {
            flags = SECCOMP_FILTER_FLAG_NEW_LISTENER;
            break;
        }
    }

    auto argv = get_argv(args);

    if (set_no_new_privs()) {
        perror("prctl");
        return 1;
    }

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &prog)) {
        perror("seccomp");
        return 1;
    }

    execvp(argv[0], argv);

    return 1;
}
