#include "Logger.h"

#include <iostream>

CSVLogger::CSVLogger(const std::string& path, const std::string& header) : out(path) {
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku logu: " << path << "\n";
        return;
    }
    if (!header.empty()) {
        out << header << "\n";
    }
}

CSVLogger::~CSVLogger() {
    if (out.is_open()) {
        out.close();
    }
}

void CSVLogger::logRow(const std::string& row) {
    if (!out.is_open()) {
        return;
    }
    out << row << "\n";
}

bool CSVLogger::ok() const {
    return out.is_open();
}
