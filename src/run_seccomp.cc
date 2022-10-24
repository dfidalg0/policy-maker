#include <run_seccomp.hh>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <iostream>
#include <wait.h>

static int alloc_notif_buffers(
    seccomp_notif **req,
    seccomp_notif_resp **res,
    seccomp_notif_sizes *sizes
);

static char *const *get_argv(std::vector<std::string> &args);

int run_seccomp(sock_fprog &prog, std::vector<std::string> &args) {
    using std::cerr;
    using std::endl;

    bool colors = isatty(STDERR_FILENO);

    int flags = has_notify_return(prog)
        ? SECCOMP_FILTER_FLAG_NEW_LISTENER
        : 0;

    auto argv = get_argv(args);

    auto const install = [&flags, &prog]() {
        return seccomp(SECCOMP_SET_MODE_FILTER, flags, &prog);
    };

    if (!flags) {
        if (set_no_new_privs()) {
            perror("prctl");
            return 1;
        }

        if (install()) {
            perror("seccomp");
            return 1;
        }

        execvp(argv[0], argv);

        return 1;
    }

    cerr
        << (colors ? "\033[31m" : "")
        << "Warning: notify return detected. All notified syscalls will be logged to stderr and handled by the kernel."
        << (colors ? "\033[0m" : "")
        << endl;

    auto pid = fork();

    if (pid == 0) {
        if (set_no_new_privs()) {
            perror("prctl");
            return 1;
        }

        auto fd = install();

        printf("%d", fd);

        if (fd < 0) {
            perror("seccomp");
            return 1;
        }

        execvp(argv[0], argv);

        return 1;
    }

    int pidfd = syscall(__NR_pidfd_open, pid, 0);

    if (pidfd < 0) {
        perror("pidfd_open");
        return 1;
    }

    int fd = -1;

    while (fd < 0) fd = syscall(__NR_pidfd_getfd, pidfd, 3, 0);

    struct sigaction sa;

    sa.sa_handler = [](int) {
        int status;
        wait(&status);
        exit(WEXITSTATUS(status));
    };

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    struct seccomp_notif_sizes sizes;
    struct seccomp_notif *req;
    struct seccomp_notif_resp *res;

    alloc_notif_buffers(&req, &res, &sizes);

    while (1) {
        // printf("Waiting for notification...\n");
        memset(req, 0, sizes.seccomp_notif);

        if (ioctl(fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            if (errno == EINTR)
                continue;
            perror("ioctl - SECCOMP_IOCTL_NOTIF_RECV");
            exit(1);
        }

        struct seccomp_data *data = &req->data;

        cerr
            << (colors ? "\033[33m" : "")
            << "Got syscall " << data->nr << " with parameters ";

        for (int i = 0; i < 5; i++) {
            cerr << data->args[i] << ", ";
        }

        cerr << data->args[5] << (colors ? "\033[0m" : "") << endl;

        res->id = req->id;
        res->error = res->val = 0;
        res->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;

        if (ioctl(fd, SECCOMP_IOCTL_NOTIF_SEND, res) == -1) {
            perror("\tS: ioctl-SECCOMP_IOCTL_NOTIF_SEND");
            exit(1);
        }
    }
}

static char *const *get_argv(std::vector<std::string> &args) {
    auto size = args.size();

    const char **argv = new const char *[size + 1];

    for (uint i = 0; i < size; ++i) {
        argv[i] = args[i].c_str();
    }

    argv[size] = nullptr;

    return (char *const *)argv;
}

static int alloc_notif_buffers(
    seccomp_notif **req,
    seccomp_notif_resp **res,
    seccomp_notif_sizes *sizes
) {
    if (seccomp(SECCOMP_GET_NOTIF_SIZES, 0, sizes) == -1)
        return -1;

    *req = (seccomp_notif *) malloc(sizes->seccomp_notif);

    if (*req == nullptr) return -1;

    size_t resp_size = sizes->seccomp_notif_resp;

    if (sizeof(seccomp_notif_resp) > resp_size)
        resp_size = sizeof(seccomp_notif_resp);

    *res = (seccomp_notif_resp *) malloc(resp_size);

    if (res == nullptr) return -1;

    return 0;
}
