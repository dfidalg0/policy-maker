#include <utils/seccomp.hh>

int seccomp(unsigned int operation, unsigned int flags, void *args) {
    return syscall(SYS_seccomp, operation, flags, args);
}

int set_no_new_privs() {
    return prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
}
