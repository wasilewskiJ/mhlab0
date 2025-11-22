// Prosty logger zapisujący dane do plików CSV.
#pragma once

#include <fstream>
#include <string>

// Klasa CSVLogger otwiera plik i pozwala dopisać wiersze.
class CSVLogger {
  public:
    // Konstruktor otwierający plik i zapisujący opcjonalny nagłówek.
    CSVLogger(const std::string& path, const std::string& header);
    // Destruktor zamyka plik.
    ~CSVLogger();
    // Dodaje jeden wiersz tekstu (już sformatowany CSV).
    void logRow(const std::string& row);
    // Sprawdza czy plik jest gotowy do zapisu.
    bool ok() const;

  private:
    std::ofstream out;  // strumień wyjściowy
};
