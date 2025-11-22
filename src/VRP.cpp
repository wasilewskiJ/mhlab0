#include "VRP.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

static double euclideanDistance(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    double dist = std::sqrt(dx * dx + dy * dy);
    return std::round(dist);
}

static std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

Problem parseVRP(const std::string& path) {
    Problem problem{};
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Nie można otworzyć pliku VRP: " + path);
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.find("DIMENSION") != std::string::npos) {
            std::stringstream ss(line);
            std::string dummy; char colon;
            ss >> dummy >> colon >> problem.dimension;
        }
        if (line.find("CAPACITY") != std::string::npos) {
            std::stringstream ss(line);
            std::string dummy; char colon;
            ss >> dummy >> colon >> problem.capacity;
        }
        if (line.find("NODE_COORD_SECTION") != std::string::npos) {
            break;
        }
    }
    std::unordered_map<int, std::pair<double, double>> coords;
    while (std::getline(in, line)) {
        if (line.find("DEMAND_SECTION") != std::string::npos) break;
        if (line.empty()) continue;
        std::stringstream ss(line);
        int id; double x, y;
        ss >> id >> x >> y;
        coords[id] = {x, y};
    }
    std::unordered_map<int, int> demands;
    while (std::getline(in, line)) {
        if (line.find("DEPOT_SECTION") != std::string::npos) break;
        if (line.empty()) continue;
        std::stringstream ss(line);
        int id, dem;
        ss >> id >> dem;
        demands[id] = dem;
    }
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        int depot;
        ss >> depot;
        if (depot == -1) break;
        problem.depotId = depot;
        break;
    }
    if (problem.depotId == 0) problem.depotId = 1;

    problem.nodes.reserve(problem.dimension);
    for (int id = 1; id <= problem.dimension; ++id) {
        double x = coords.count(id) ? coords[id].first : 0.0;
        double y = coords.count(id) ? coords[id].second : 0.0;
        int dem = demands.count(id) ? demands[id] : 0;
        problem.nodes.push_back(Node{id, x, y, dem});
    }
    problem.distances.assign(problem.dimension + 1, std::vector<double>(problem.dimension + 1, 0.0));
    for (int i = 1; i <= problem.dimension; ++i) {
        for (int j = 1; j <= problem.dimension; ++j) {
            const Node& a = problem.nodes[i - 1];
            const Node& b = problem.nodes[j - 1];
            problem.distances[i][j] = euclideanDistance(a.x, a.y, b.x, b.y);
        }
    }
    return problem;
}

double readOptimalCost(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return -1.0;
    }
    std::string line;
    double value = -1.0;
    while (std::getline(in, line)) {
        if (line.find("Cost") != std::string::npos) {
            std::stringstream ss(line);
            std::string label;
            ss >> label;
            ss >> value;
        }
    }
    return value;
}

double evaluateSolution(const Problem& problem, const Solution& solution) {
    double total = 0.0;
    for (const auto& route : solution.routes) {
        int prev = problem.depotId;
        for (int nodeId : route) {
            total += problem.distances[prev][nodeId];
            prev = nodeId;
        }
        total += problem.distances[prev][problem.depotId];
    }
    return total;
}

Solution decodePermutation(const Problem& problem, const std::vector<int>& permutation) {
    Solution sol;
    sol.cost = 0.0;
    std::vector<int> currentRoute;
    int currentLoad = 0;
    for (int customer : permutation) {
        int demand = problem.nodes[customer - 1].demand;
        if (currentLoad + demand > problem.capacity) {
            sol.routes.push_back(currentRoute);
            currentRoute.clear();
            currentLoad = 0;
        }
        currentRoute.push_back(customer);
        currentLoad += demand;
    }
    if (!currentRoute.empty()) {
        sol.routes.push_back(currentRoute);
    }
    sol.cost = evaluateSolution(problem, sol);
    return sol;
}

std::vector<int> randomPermutation(const Problem& problem) {
    std::vector<int> perm;
    perm.reserve(problem.dimension - 1);
    for (const auto& node : problem.nodes) {
        if (node.id == problem.depotId) {
            continue;
        }
        perm.push_back(node.id);
    }
    std::shuffle(perm.begin(), perm.end(), rng());
    return perm;
}
