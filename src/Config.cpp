// Prosty loader konfiguracji z pliku key=value.
#include "Config.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

static std::string trim(const std::string& text) {
    size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

Config ConfigLoader::load(const std::string& path) {
    readFile(path);
    return buildConfig(path);
}

void ConfigLoader::readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Nie można otworzyć pliku konfiguracyjnego: " + path);
    }
    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }
        size_t eqPos = trimmed.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }
        std::string key = trim(trimmed.substr(0, eqPos));
        std::string value = trim(trimmed.substr(eqPos + 1));
        kv[key] = value;
    }
}

Config ConfigLoader::buildConfig(const std::string& path) {
    Config cfg;
    (void)path;
    cfg.inputDirectory = getString("input_directory", "inputs");
    cfg.optimalDirectory = getString("optimal_directory", "optimal-solutions");
    cfg.logDir = getString("log_dir", "logs");
    cfg.randomRuns = getInt("random_runs", 10000);
    cfg.greedyRuns = getInt("greedy_runs", 0);
    cfg.saRuns = getInt("sa_runs", 10);
    cfg.eaRuns = getInt("ea_runs", 10);
    cfg.randomIterations = getInt("random_iterations", 1000);
    cfg.greedyRestarts = getInt("greedy_restarts", 32);
    cfg.saInitialTemp = getDouble("sa_initial_temp", 100.0);
    cfg.saMinTemp = getDouble("sa_min_temp", 0.01);
    cfg.saCoolingRate = getDouble("sa_cooling_rate", 0.995);
    cfg.saIterations = getInt("sa_iterations_per_temp", 200);
    cfg.eaPopulation = getInt("ea_population", 100);
    cfg.eaGenerations = getInt("ea_generations", 100);
    cfg.eaCrossoverRate = getDouble("ea_crossover_rate", 0.7);
    cfg.eaMutationRate = getDouble("ea_mutation_rate", 0.1);
    cfg.eaTournament = getInt("ea_tournament", 5);
    cfg.eaElites = getInt("ea_elites", 1);
    cfg.verbose = getBool("verbose", true);
    return cfg;
}

std::string ConfigLoader::getString(const std::string& key, const std::string& def) {
    auto it = kv.find(key);
    if (it == kv.end()) {
        return def;
    }
    return it->second;
}

int ConfigLoader::getInt(const std::string& key, int def) {
    auto it = kv.find(key);
    if (it == kv.end()) {
        return def;
    }
    return std::stoi(it->second);
}

double ConfigLoader::getDouble(const std::string& key, double def) {
    auto it = kv.find(key);
    if (it == kv.end()) {
        return def;
    }
    return std::stod(it->second);
}

bool ConfigLoader::getBool(const std::string& key, bool def) {
    auto it = kv.find(key);
    if (it == kv.end()) {
        return def;
    }
    std::string v = it->second;
    if (v == "1" || v == "true" || v == "True" || v == "TRUE") {
        return true;
    }
    if (v == "0" || v == "false" || v == "False" || v == "FALSE") {
        return false;
    }
    return def;
}
