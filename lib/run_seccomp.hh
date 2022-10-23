#ifndef __RUN_SECCOMP_HH__
#define __RUN_SECCOMP_HH__

#include <utils/seccomp.hh>
#include <utils/cmd_parse.hh>
#include <vector>
#include <string>

int run_seccomp(sock_fprog& prog, std::vector<std::string>& args);

#endif // __RUN_SECCOMP_HH__
