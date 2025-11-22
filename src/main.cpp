#include "Algorithms.h"
#include "Config.h"
#include "Logger.h"
#include "Stats.h"
#include "VRP.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::string configPath = "config.ini";
    if (argc > 1) {
        configPath = argv[1];
    }

    ConfigLoader loader;
    Config cfg;
    try {
        cfg = loader.load(configPath);
    } catch (const std::exception& ex) {
        std::cerr << "Błąd konfiguracji: " << ex.what() << "\n";
        return 1;
    }
    std::filesystem::create_directories(cfg.logDir);

    std::vector<std::string> summaryCsv;
    summaryCsv.push_back("instance,optimal,random_runs,random_best,random_worst,random_avg,random_std,greedy_runs,greedy_best,greedy_worst,greedy_avg,greedy_std,ea_runs,ea_best,ea_worst,ea_avg,ea_std,sa_runs,sa_best,sa_worst,sa_avg,sa_std");

    for (const auto& entry : std::filesystem::directory_iterator(cfg.inputDirectory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".vrp") {
            continue;
        }
        std::string vrpPath = entry.path().string();
        std::string baseName = entry.path().stem().string();
        std::string optPath = (std::filesystem::path(cfg.optimalDirectory) / (baseName + ".sol")).string();
        Problem problem;
        try {
            problem = parseVRP(vrpPath);
        } catch (const std::exception& ex) {
            std::cerr << "Błąd wczytywania VRP (" << vrpPath << "): " << ex.what() << "\n";
            continue;
        }
        double optimalCost = readOptimalCost(optPath);
        std::filesystem::path instLogDir = std::filesystem::path(cfg.logDir) / baseName;
        std::filesystem::create_directories(instLogDir);

        int randomRuns = cfg.randomRuns;
        int greedyRuns = cfg.greedyRuns > 0 ? cfg.greedyRuns : problem.dimension;
        int eaRuns = cfg.eaRuns;
        int saRuns = cfg.saRuns;

        std::vector<double> randomScores;
        std::vector<double> greedyScores;
        std::vector<double> saScores;
        std::vector<double> eaScores;

        for (int run = 0; run < randomRuns; ++run) {
            std::string logPath = (instLogDir / ("random_run_" + std::to_string(run) + ".csv")).string();
            CSVLogger logger(logPath, "iteration,best,current,avg,worst");
            Solution bestSol = runRandomSearch(problem, cfg.randomIterations, logger);
            randomScores.push_back(bestSol.cost);
        }

        for (int run = 0; run < greedyRuns; ++run) {
            std::string logPath = (instLogDir / ("greedy_run_" + std::to_string(run) + ".csv")).string();
            CSVLogger logger(logPath, "restart,best,current,avg,worst");
            Solution bestSol = runGreedy(problem, cfg.greedyRestarts, logger);
            greedyScores.push_back(bestSol.cost);
        }

        for (int run = 0; run < saRuns; ++run) {
            std::string logPath = (instLogDir / ("sa_run_" + std::to_string(run) + ".csv")).string();
            CSVLogger logger(logPath, "step,best,current,avg,worst");
            Solution bestSol = runSimulatedAnnealing(problem, cfg, logger);
            saScores.push_back(bestSol.cost);
        }

        for (int run = 0; run < eaRuns; ++run) {
            std::string logPath = (instLogDir / ("ea_run_" + std::to_string(run) + ".csv")).string();
            CSVLogger logger(logPath, "generation,best,avg,worst");
            Solution bestSol = runEvolutionary(problem, cfg, logger);
            eaScores.push_back(bestSol.cost);
        }

        RunStats randomStats = computeStats(randomScores);
        RunStats greedyStats = computeStats(greedyScores);
        RunStats saStats = computeStats(saScores);
        RunStats eaStats = computeStats(eaScores);

        std::cout << "Instancja: " << baseName << " (" << vrpPath << ")\n";
        if (optimalCost > 0) {
            std::cout << "Optymalny koszt (z pliku): " << optimalCost << "\n";
        }
        auto printStats = [](const std::string& name, const RunStats& s) {
            std::cout << name << " -> best: " << s.best << ", worst: " << s.worst
                      << ", avg: " << s.avg << ", std: " << s.std << "\n";
        };
        printStats("Losowy", randomStats);
        printStats("Zachlanny", greedyStats);
        printStats("SA", saStats);
        printStats("EA", eaStats);
        std::cout << "\n";

        std::ostringstream csvRow;
        csvRow << baseName << "," << optimalCost << ","
               << randomRuns << "," << randomStats.best << "," << randomStats.worst << "," << randomStats.avg << "," << randomStats.std << ","
               << greedyRuns << "," << greedyStats.best << "," << greedyStats.worst << "," << greedyStats.avg << "," << greedyStats.std << ","
               << eaRuns << "," << eaStats.best << "," << eaStats.worst << "," << eaStats.avg << "," << eaStats.std << ","
               << saRuns << "," << saStats.best << "," << saStats.worst << "," << saStats.avg << "," << saStats.std;
        summaryCsv.push_back(csvRow.str());

    }

    std::ofstream csvFile(std::filesystem::path(cfg.logDir) / "summary.csv");
    if (csvFile.is_open()) {
        for (const auto& row : summaryCsv) {
            csvFile << row << "\n";
        }
    }
    return 0;
}
