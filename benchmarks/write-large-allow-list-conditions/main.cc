#include <unistd.h>

int times = 10'000'000;

static const char msg[] = "Hello, World\n";

void benchmark() {
    write(1, msg, sizeof(msg));
}
