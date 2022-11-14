#include <unistd.h>

int times = 1'000'000;

void benchmark() {
    const char msg[] = "Hello, World\n";
    write(1, msg, sizeof(msg));
}
