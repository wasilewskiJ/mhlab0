// Deklaracje wszystkich zaimplementowanych algorytmów poszukiwań.
#pragma once

#include "Config.h"
#include "VRP.h"
#include "Logger.h"

// Uruchamia algorytm losowego przeszukiwania przez podaną liczbę iteracji.
Solution runRandomSearch(const Problem& problem, int iterations, CSVLogger& logger);

// Uruchamia algorytm zachłanny wielokrotnie (różne starty) i zwraca najlepsze znalezione rozwiązanie.
Solution runGreedy(const Problem& problem, int restarts, CSVLogger& logger);

// Uruchamia symulowane wyżarzanie zgodnie z parametrami z Config.
Solution runSimulatedAnnealing(const Problem& problem, const Config& cfg, CSVLogger& logger);

// Uruchamia algorytm ewolucyjny zgodnie z parametrami z Config.
Solution runEvolutionary(const Problem& problem, const Config& cfg, CSVLogger& logger);
