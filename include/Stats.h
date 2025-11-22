// Proste funkcje do liczenia metryk statystycznych.
#pragma once

#include <vector>

// Struktura gromadząca wartości best/worst/avg/std.
struct RunStats {
    double best;  // najlepszy wynik (min)
    double worst; // najgorszy wynik (max)
    double avg;
    double std;
};

// Funkcja liczy statystyki dla podanego wektora wyników (niższy = lepszy).
RunStats computeStats(const std::vector<double>& values);
