#include <utils/seccomp.hh>

int seccomp(unsigned int operation, unsigned int flags, void *args) {
    return syscall(SYS_seccomp, operation, flags, args);
}

int set_no_new_privs() {
    return prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
}

bool has_notify_return(const sock_fprog& prog) {
    for (int i = 0; i < prog.len; i++) {
        auto filter = prog.filter[i];

        if (BPF_CLASS(filter.code) == BPF_RET && filter.k == SECCOMP_RET_USER_NOTIF) {
            return true;
        }
    }

    return false;
}
