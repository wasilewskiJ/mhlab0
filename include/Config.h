// Prosty nagłówek z definicją struktury konfiguracji i loadera z pliku ini.
#pragma once

#include <string>
#include <unordered_map>

// Struktura Config przechowuje wszystkie parametry sterujące programem.
struct Config {
    // Katalog z plikami wejściowymi VRP.
    std::string inputDirectory;
    // Katalog z plikami z optymalnymi rozwiązaniami.
    std::string optimalDirectory;
    // Katalog do zapisywania logów CSV.
    std::string logDir;
    // Liczba uruchomień algorytmu losowego (statystyki między uruchomieniami).
    int randomRuns;
    // Liczba uruchomień algorytmu zachłannego (0 oznacza: użyj liczby miast).
    int greedyRuns;
    // Liczba uruchomień SA.
    int saRuns;
    // Liczba uruchomień EA.
    int eaRuns;
    // Iteracje algorytmu losowego.
    int randomIterations;
    // Liczba restartów algorytmu zachłannego.
    int greedyRestarts;
    // Parametry SA: temperatura początkowa.
    double saInitialTemp;
    // Parametry SA: minimalna temperatura zatrzymania.
    double saMinTemp;
    // Parametry SA: współczynnik chłodzenia.
    double saCoolingRate;
    // Parametry SA: iteracji na jedną temperaturę.
    int saIterations;
    // Parametry EA: rozmiar populacji.
    int eaPopulation;
    // Parametry EA: liczba pokoleń.
    int eaGenerations;
    // Parametry EA: prawdopodobieństwo krzyżowania.
    double eaCrossoverRate;
    // Parametry EA: prawdopodobieństwo mutacji.
    double eaMutationRate;
    // Parametry EA: rozmiar turnieju selekcji.
    int eaTournament;
    // Parametry EA: liczba osobników elitarnych.
    int eaElites;
    // Flaga pozwalająca na logowanie rozbudowane.
    bool verbose;
};

// Prosta klasa wczytująca plik konfiguracyjny w formacie key=value.
class ConfigLoader {
  public:
    // Konstruktor nic nie robi.
    ConfigLoader() = default;
    // Metoda load wczytuje plik i wypełnia strukturę Config domyślnymi i podanymi wartościami.
    Config load(const std::string& path);

  private:
    // Pomocnicza mapa trzymająca pary klucz-wartość jako tekst.
    std::unordered_map<std::string, std::string> kv;
    // Wewnętrzna metoda czytająca plik do mapy.
    void readFile(const std::string& path);
    // Wewnętrzna metoda ustawiająca wartości w strukturze Config z mapy i domyślnych nastaw.
    Config buildConfig(const std::string& path);
    // Pomocnicza metoda zwracająca wartość tekstową lub domyślną.
    std::string getString(const std::string& key, const std::string& def);
    // Pomocnicza metoda zwracająca wartość całkowitą lub domyślną.
    int getInt(const std::string& key, int def);
    // Pomocnicza metoda zwracająca wartość zmiennoprzecinkową lub domyślną.
    double getDouble(const std::string& key, double def);
    // Pomocnicza metoda zwracająca wartość boolowską lub domyślną (true dla "1"/"true").
    bool getBool(const std::string& key, bool def);
};
