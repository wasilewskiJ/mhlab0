// Definicje struktur reprezentujących problem cVRP i funkcje pomocnicze.
#pragma once

#include <string>
#include <vector>
// Pojedynczy węzeł (lokalizacja) z zapotrzebowaniem i współrzędnymi.
struct Node {
    int id;          // identyfikator węzła (1..N)
    double x;        // współrzędna X
    double y;        // współrzędna Y
    int demand;      // zapotrzebowanie
};

// Struktura opisująca cały problem cVRP.
struct Problem {
    int dimension;                      // liczba węzłów
    int capacity;                       // pojemność pojazdu
    int depotId;                        // identyfikator depo (zwykle 1)
    std::vector<Node> nodes;            // lista węzłów
    std::vector<std::vector<double>> distances;  // macierz odległości
};

// Rozwiązanie składa się z tras oraz kosztu.
struct Solution {
    std::vector<std::vector<int>> routes;  // każda trasa to lista węzłów bez depo
    double cost;                           // łączny koszt tras
};

// Funkcja wczytuje plik VRP i buduje strukturę Problem.
Problem parseVRP(const std::string& path);

// Funkcja wczytuje linię "Cost xx" z pliku optimum, zwraca -1 jeśli brak.
double readOptimalCost(const std::string& path);

// Funkcja liczy koszt pełnej trasy (start i powrót do depo dla każdej sekwencji).
double evaluateSolution(const Problem& problem, const Solution& solution);

// Funkcja przelicza permutację klientów na trasy zgodnie z ograniczeniami pojemności.
Solution decodePermutation(const Problem& problem, const std::vector<int>& permutation);

// Funkcja generuje losową permutację klientów (bez depo).
std::vector<int> randomPermutation(const Problem& problem);
