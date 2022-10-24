#ifndef __UTILS_SECCOMP_HH__
#define __UTILS_SECCOMP_HH__

#include <syscall.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <sys/prctl.h>

int seccomp(unsigned int operation, unsigned int flags, void *args);

int set_no_new_privs();

bool has_notify_return(const sock_fprog& prog);

#endif // __UTILS_SECCOMP_HH__
