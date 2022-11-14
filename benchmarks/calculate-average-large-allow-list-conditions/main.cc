#include <iostream>

int times = 1'000'000;

void benchmark() {
    int sum = 0;

    for (int i = 0; i < 500; ++i) {
        sum += rand() % 100;
    }

    std::cout << sum / 500 << std::endl;
}
