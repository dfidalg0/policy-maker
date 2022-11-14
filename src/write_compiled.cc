#include <write_compiled.hh>
#include <errors.hh>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

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

struct File {
    std::string name;
    std::string extension;
};

File get_name_and_extension(const std::string &filename) {
    auto idx = filename.rfind(".");

    if (idx == std::string::npos) {
        throw std::runtime_error("Invalid filename");
    }

    return {
        .name = filename.substr(0, idx),
        .extension = filename.substr(1 + idx)
    };
}

void write_compiled(std::string pathname, std::string target, CompileResult &result) {
    std::regex re(R"([a-zA-Z_][a-zA-Z0-9_]*)");

    if (!std::regex_match(target, re)) {
        throw std::runtime_error("Invalid function name: " + target);
    }

    std::string filename = fs::path(pathname).filename();

    if (filename.empty()) {
        filename = "filter.c";
        pathname = fs::path(pathname).append(filename).string();
    }

    std::ofstream file(pathname);

    auto [name, ext] = get_name_and_extension(pathname);

    std::string header_ext =
        (ext == "cp" || ext == "cpp" || ext == "cc" || ext == "c++")
            ? ".hh"
            : ".h";

    auto header_pathname = name + header_ext;
    std::string header_filename = fs::path(header_pathname).filename();

    std::ofstream header(header_pathname);

    if (!file.is_open()) {
        throw FileNotFoundError(pathname);
    }

    if (!header.is_open()) {
        throw FileNotFoundError(header_pathname);
    }

    std::string flag = "0";
    std::string fail_condition = "ret";
    std::string return_doc = "0 when the filter is installed successfully.";

    if (has_notify_return((sock_fprog) result)) {
        flag = "SECCOMP_FILTER_FLAG_NEW_LISTENER";
        fail_condition = "ret < 0";
        return_doc = "a file descriptor where notifications can be read from, or -1 on error.";
    }

    Wrapper(header)
        << "#ifndef __POLICY_FILTER_GENERATED__"
        << "#define __POLICY_FILTER_GENERATED__"
        << ""
        << "int " + target + "();"
        << ""
        << "#endif // __POLICY_FILTER_GENERATED__";

    header.close();

    auto file_wrapper = Wrapper(file);

    file_wrapper
        << "/**"
        << " * In order to use the " + target + " utility, you must compile this file"
        << " * and include the line:"
        << " * extern int " + target + "();"
        << " * in your program or"
        << " * include the " + header_filename + " generated header"
        << " */"
        << ""
        << "#include <linux/filter.h>"
        << "#include <unistd.h>"
        << "#include <sys/syscall.h>"
        << "#include <linux/seccomp.h>"
        << "#include <sys/prctl.h>"
        << "#include <stdio.h>"
        << "#include \"" + header_filename + "\""
        << ""
        << "static int seccomp(unsigned int operation, unsigned int flags, void *args) {"
        << "    return syscall(__NR_seccomp, operation, flags, args);"
        << "}"
        << ""
        << "/**"
        << " * Install the seccomp filter in the current process."
        << " * @returns " + return_doc
        << " */"
        << "int " + target + "() {"
        << "    /* If your editor supports folding, this is a good place to use it. */"
        << "    static struct sock_filter filter[] = {";

    file << result.to_string(8);

    file_wrapper
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
