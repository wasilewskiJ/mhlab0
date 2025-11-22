#include "Random.h"

#include <random>

std::mt19937& globalRng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

int randInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(globalRng());
}

double randUnit() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(globalRng());
}
