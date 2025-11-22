// Implementacje algorytmów: losowy, zachłanny, SA, EA.
#include "Algorithms.h"

#include "Random.h"
#include "Stats.h"
#include "VRP.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

// Reprezentacja osobnika dla EA.
struct Individual {
    std::vector<int> perm;
    double cost;
};

// Buduje permutację metodą najbliższego sąsiada startując z podanego węzła.
static std::vector<int> buildGreedyPermutation(const Problem& problem, int startId) {
    std::unordered_set<int> unvisited;
    for (const auto& node : problem.nodes) {
        if (node.id != problem.depotId) unvisited.insert(node.id);
    }
    std::vector<int> order;
    order.reserve(problem.dimension - 1);
    int current = unvisited.count(startId) ? startId : *unvisited.begin();
    while (!unvisited.empty()) {
        order.push_back(current);
        unvisited.erase(current);
        if (unvisited.empty()) break;
        double bestDist = std::numeric_limits<double>::infinity();
        int bestNext = *unvisited.begin();
        for (int candidate : unvisited) {
            double dist = problem.distances[current][candidate];
            if (dist < bestDist) {
                bestDist = dist;
                bestNext = candidate;
            }
        }
        current = bestNext;
    }
    return order;
}

// Tworzy sąsiada przez zamianę dwóch losowych pozycji w permutacji.
static std::vector<int> swapNeighbor(const std::vector<int>& perm) {
    std::vector<int> result = perm;
    if (result.size() < 2) return result;
    int i = randInt(0, static_cast<int>(result.size()) - 1);
    int j = randInt(0, static_cast<int>(result.size()) - 1);
    while (j == i) j = randInt(0, static_cast<int>(result.size()) - 1);
    std::swap(result[i], result[j]);
    return result;
}

// Wielokrotne losowe próbkowanie permutacji; zwraca najlepszą znalezioną.
Solution runRandomSearch(const Problem& problem, int iterations, CSVLogger& logger) {
    Solution bestSolution;
    bestSolution.cost = std::numeric_limits<double>::infinity();
    double sumCost = 0.0;
    double worstCost = -std::numeric_limits<double>::infinity();
    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<int> perm = randomPermutation(problem);
        Solution sol = decodePermutation(problem, perm);
        sumCost += sol.cost;
        if (sol.cost < bestSolution.cost) bestSolution = sol;
        if (sol.cost > worstCost) worstCost = sol.cost;
        double avgCost = sumCost / static_cast<double>(iter + 1);
        logger.logRow(std::to_string(iter) + "," + std::to_string(bestSolution.cost) + "," +
                      std::to_string(sol.cost) + "," + std::to_string(avgCost) + "," +
                      std::to_string(worstCost));
    }
    return bestSolution;
}

// Pojedynczy bieg greedy od wybranego startu.
static Solution greedyOnce(const Problem& problem, int startId) {
    std::vector<int> perm = buildGreedyPermutation(problem, startId);
    return decodePermutation(problem, perm);
}

// Wiele restartów greedy; loguje postęp i zwraca najlepszy wynik.
Solution runGreedy(const Problem& problem, int restarts, CSVLogger& logger) {
    Solution bestSolution;
    bestSolution.cost = std::numeric_limits<double>::infinity();
    double worstCost = -std::numeric_limits<double>::infinity();
    double sumCost = 0.0;
    for (int r = 0; r < restarts; ++r) {
        int startId = 2 + (r % (problem.dimension - 1));
        Solution sol = greedyOnce(problem, startId);
        sumCost += sol.cost;
        if (sol.cost < bestSolution.cost) bestSolution = sol;
        if (sol.cost > worstCost) worstCost = sol.cost;
        double avgCost = sumCost / static_cast<double>(r + 1);
        logger.logRow(std::to_string(r) + "," + std::to_string(bestSolution.cost) + "," +
                      std::to_string(sol.cost) + "," + std::to_string(avgCost) + "," +
                      std::to_string(worstCost));
    }
    return bestSolution;
}

// Symulowane wyżarzanie z sąsiedztwem swap i stałym chłodzeniem.
Solution runSimulatedAnnealing(const Problem& problem, const Config& cfg, CSVLogger& logger) {
    int randomStart = 2 + (randInt(0, problem.dimension - 2));
    std::vector<int> startPerm = buildGreedyPermutation(problem, randomStart);
    Solution currentSol = decodePermutation(problem, startPerm);
    Solution bestSol = currentSol;
    double temp = cfg.saInitialTemp;
    double worstCost = currentSol.cost;
    double sumCost = currentSol.cost;
    int steps = 1;
    int iterationCounter = 0;
    while (temp > cfg.saMinTemp) {
        for (int k = 0; k < cfg.saIterations; ++k) {
            std::vector<int> neighborPerm = swapNeighbor(startPerm);
            Solution neighborSol = decodePermutation(problem, neighborPerm);
            double delta = neighborSol.cost - currentSol.cost;
            bool accept = delta < 0 || randUnit() < std::exp(-delta / temp);
            if (accept) {
                startPerm = neighborPerm;
                currentSol = neighborSol;
            }
            if (currentSol.cost < bestSol.cost) bestSol = currentSol;
            if (currentSol.cost > worstCost) worstCost = currentSol.cost;
            sumCost += currentSol.cost;
            steps += 1;
            double avgCost = sumCost / static_cast<double>(steps);
            logger.logRow(std::to_string(iterationCounter) + "," + std::to_string(bestSol.cost) + "," +
                          std::to_string(currentSol.cost) + "," + std::to_string(avgCost) + "," +
                          std::to_string(worstCost));
            iterationCounter += 1;
        }
        temp *= cfg.saCoolingRate;
    }
    return bestSol;
}

// Krzyżowanie OX: segment z p1, reszta w kolejności p2, bez duplikatów.
static std::vector<int> orderedCrossover(const std::vector<int>& p1, const std::vector<int>& p2) {
    int n = static_cast<int>(p1.size());
    std::vector<int> child(n, -1);
    int a = randInt(0, n - 1);
    int b = randInt(0, n - 1);
    if (a > b) std::swap(a, b);
    for (int i = a; i <= b; ++i) child[i] = p1[i];
    int idx = (b + 1) % n;
    for (int i = 0; i < n; ++i) {
        int candidate = p2[(b + 1 + i) % n];
        bool used = false;
        for (int val : child) if (val == candidate) { used = true; break; }
        if (used) continue;
        while (child[idx] != -1) idx = (idx + 1) % n;
        child[idx] = candidate;
    }
    return child;
}

// Mutacja swap z prawdopodobieństwem Pm.
static void mutateSwap(std::vector<int>& perm, double mutationRate) {
    if (perm.size() < 2) return;
    if (randUnit() < mutationRate) {
        int i = randInt(0, static_cast<int>(perm.size()) - 1);
        int j = randInt(0, static_cast<int>(perm.size()) - 1);
        while (j == i) j = randInt(0, static_cast<int>(perm.size()) - 1);
        std::swap(perm[i], perm[j]);
    }
}

// Selekcja turniejowa, zwraca indeks najlepszego z wylosowanych kandydatów.
static int tournamentSelect(const std::vector<Individual>& pop, int tourSize) {
    int bestIdx = -1;
    double bestCost = std::numeric_limits<double>::infinity();
    for (int i = 0; i < tourSize; ++i) {
        int idx = randInt(0, static_cast<int>(pop.size()) - 1);
        if (pop[idx].cost < bestCost) {
            bestCost = pop[idx].cost;
            bestIdx = idx;
        }
    }
    return bestIdx;
}

// Algorytm ewolucyjny: inicjalizacja losowa, turniej, OX, mutacja swap, elity.
Solution runEvolutionary(const Problem& problem, const Config& cfg, CSVLogger& logger) {
    std::vector<Individual> population;
    population.reserve(cfg.eaPopulation);
    for (int i = 0; i < cfg.eaPopulation; ++i) {
        std::vector<int> perm = randomPermutation(problem);
        Solution sol = decodePermutation(problem, perm);
        population.push_back(Individual{perm, sol.cost});
    }
    Individual bestOverall = population[0];
    for (const auto& ind : population) if (ind.cost < bestOverall.cost) bestOverall = ind;

    for (int gen = 0; gen < cfg.eaGenerations; ++gen) {
        double bestCost = std::numeric_limits<double>::infinity();
        double worstCost = -std::numeric_limits<double>::infinity();
        double sumCost = 0.0;
        for (const auto& ind : population) {
            bestCost = std::min(bestCost, ind.cost);
            worstCost = std::max(worstCost, ind.cost);
            sumCost += ind.cost;
        }
        double avgCost = sumCost / static_cast<double>(population.size());
        logger.logRow(std::to_string(gen) + "," + std::to_string(bestCost) + "," +
                      std::to_string(avgCost) + "," + std::to_string(worstCost));
        if (bestCost < bestOverall.cost) {
            for (const auto& ind : population) if (ind.cost == bestCost) { bestOverall = ind; break; }
        }

        std::vector<Individual> newPop;
        newPop.reserve(cfg.eaPopulation);
        std::vector<Individual> sortedPop = population;
        std::sort(sortedPop.begin(), sortedPop.end(), [](const Individual& a, const Individual& b) {
            return a.cost < b.cost;
        });
        int elites = std::min(cfg.eaElites, static_cast<int>(sortedPop.size()));
        for (int e = 0; e < elites; ++e) newPop.push_back(sortedPop[e]);

        while (static_cast<int>(newPop.size()) < cfg.eaPopulation) {
            int p1Idx = tournamentSelect(population, cfg.eaTournament);
            int p2Idx = tournamentSelect(population, cfg.eaTournament);
            const auto& parent1 = population[p1Idx].perm;
            const auto& parent2 = population[p2Idx].perm;
            std::vector<int> childPerm;
            if (randUnit() < cfg.eaCrossoverRate) childPerm = orderedCrossover(parent1, parent2);
            else childPerm = parent1;
            mutateSwap(childPerm, cfg.eaMutationRate);
            Solution childSol = decodePermutation(problem, childPerm);
            newPop.push_back(Individual{childPerm, childSol.cost});
        }
        population = std::move(newPop);
    }
    return decodePermutation(problem, bestOverall.perm);
}
