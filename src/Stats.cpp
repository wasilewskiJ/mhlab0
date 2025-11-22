#include "Stats.h"

#include <cmath>
#include <cstddef>
#include <limits>

RunStats computeStats(const std::vector<double>& values) {
    if (values.empty()) {
        return RunStats{0.0, 0.0, 0.0, 0.0};
    }
    double best = std::numeric_limits<double>::infinity();
    double worst = -std::numeric_limits<double>::infinity();
    double sum = 0.0;
    for (double v : values) {
        if (v < best) {
            best = v;
        }
        if (v > worst) {
            worst = v;
        }
        sum += v;
    }
    double avg = sum / static_cast<double>(values.size());
    double varSum = 0.0;
    for (double v : values) {
        double diff = v - avg;
        varSum += diff * diff;
    }
    double variance = varSum / static_cast<double>(values.size());
    double stdev = std::sqrt(variance);
    return RunStats{best, worst, avg, stdev};
}
