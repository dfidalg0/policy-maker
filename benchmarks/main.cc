#include <iostream>
#include <chrono>
#include <unistd.h>
#include <wait.h>
#include <sys/mman.h>

using namespace std::chrono;

extern void benchmark();
extern int install_filter();
extern int times;

void run(bool filter) {
    long * durations = (long *) mmap(
        nullptr,
        times * sizeof(long),
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    if (fork() == 0) {
        if (filter) install_filter();

        for (auto i = 0; i < times; ++i) {
            auto start = high_resolution_clock::now();
            benchmark();
            auto end = high_resolution_clock::now();
            long duration = duration_cast<nanoseconds>(end - start).count();

            durations[i] = duration;
        }

        exit(0);
    }
    else {
        int status;
        wait(&status);
    }

    for (auto i = 0; i < times; ++i) {
        std::cerr << durations[i] << '\n';
    }

    munmap(durations, times * sizeof(long));
}

int main() {
    run(false);
    std::cerr << "---\n";
    run(true);

    return 0;
}
