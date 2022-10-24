#include <write_compiled.hh>
#include <errors.hh>

class Wrapper {
public:
    Wrapper(std::ofstream &file) {
        this->file = &file;
    }

    Wrapper& operator<< (const std::string &str) {
        *file << str << '\n';
        return *this;
    }
private:
    std::ofstream * file;
};


void write_compiled(const std::string &filename, CompileResult &result) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        throw FileNotFoundError(filename);
    }

    std::string flag = "0";
    std::string fail_condition = "ret";
    std::string return_doc = "0 when the filter is installed successfully.";

    if (has_notify_return((sock_fprog) result)) {
        flag = "SECCOMP_FILTER_FLAG_NEW_LISTENER";
        fail_condition = "ret < 0";
        return_doc = "a file descriptor where notifications can be read from, or -1 on error.";
    }

    auto wrapper = Wrapper(file);

    wrapper
        << "/**"
        << " * In order to use the install_filter utility, you must compile this file"
        << " * and include the line:"
        << " * extern int install_filter();"
        << " * in your program."
        << " */"
        << ""
        << "#include <linux/filter.h>"
        << "#include <unistd.h>"
        << "#include <sys/syscall.h>"
        << "#include <linux/seccomp.h>"
        << "#include <sys/prctl.h>"
        << "#include <stdio.h>"
        << ""
        << "static int seccomp(unsigned int operation, unsigned int flags, void *args) {"
        << "    return syscall(__NR_seccomp, operation, flags, args);"
        << "}"
        << ""
        << "/**"
        << " * Install the seccomp filter in the current process."
        << " * @returns " + return_doc
        << " */"
        << "int install_filter() {"
        << "    /* If your editor supports folding, this is a good place to use it. */"
        << "    static struct sock_filter filter[] = {";

    file << result.to_string(8);

    wrapper
        << "    };"
        << ""
        << "    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {"
        << "        perror(\"prctl\");"
        << "        return -1;"
        << "    }"
        << ""
        << "    struct sock_fprog prog = {"
        << "        .len = sizeof(filter) / sizeof(filter[0]),"
        << "        .filter = filter"
        << "    };"
        << ""
        << "    int ret = seccomp(SECCOMP_SET_MODE_FILTER, " + flag + ", &prog);"
        << ""
        << "    if (" + fail_condition + ") {"
        << "        perror(\"seccomp\");"
        << "        return -1;"
        << "    }"
        << ""
        << "    return ret;"
        << "}";

        file.close();
}
