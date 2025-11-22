// Pomocnicze funkcje losowe oparte o wspólny generator.
#pragma once

#include <random>

// Zwraca referencję do globalnego generatora.
std::mt19937& globalRng();

// Losuje liczbę całkowitą z zakresu [min, max].
int randInt(int min, int max);

// Losuje liczbę zmiennoprzecinkową z zakresu [0, 1].
double randUnit();
