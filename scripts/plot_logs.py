#!/usr/bin/env python3
# Skrypt rysuje przebiegi z logów CSV zapisanych przez program.
# Konfiguracja na początku pliku:
LOG_DIR = "logs_baseline"      # katalog z logami (per instancja podkatalog)
OUTPUT_DIR = "plots_baseline"  # katalog na wykresy
RUN_PICK_STRATEGY = "middle"   # "middle" wybiera środkowy run, można ustawić "first"/"last"/indeks int

import os  # operacje na ścieżkach
import glob  # wyszukiwanie plików
import math  # do obliczeń
import csv  # czytanie csv
from typing import List, Dict  # typowanie

import matplotlib.pyplot as plt  # rysowanie wykresów


def ensure_dir(path: str) -> None:
    # Tworzy katalog jeśli nie istnieje.
    os.makedirs(path, exist_ok=True)


def pick_run(files: List[str]) -> str:
    # Wybiera plik run według strategii.
    if not files:
        return ""
    if RUN_PICK_STRATEGY == "first":
        return sorted(files)[0]
    if RUN_PICK_STRATEGY == "last":
        return sorted(files)[-1]
    if RUN_PICK_STRATEGY == "middle":
        files_sorted = sorted(files)
        return files_sorted[len(files_sorted) // 2]
    if isinstance(RUN_PICK_STRATEGY, int):
        files_sorted = sorted(files)
        idx = max(0, min(len(files_sorted) - 1, RUN_PICK_STRATEGY))
        return files_sorted[idx]
    return sorted(files)[0]


def read_series(path: str) -> Dict[str, List[float]]:
    # Czyta kolumny numeryczne z CSV do słowników list.
    data: Dict[str, List[float]] = {}
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            for key, val in row.items():
                try:
                    v = float(val)
                except (ValueError, TypeError):
                    continue
                data.setdefault(key, []).append(v)
    return data


def plot_single(instance: str, alg_name: str, data: Dict[str, List[float]], optimal: float, out_path: str) -> None:
    # Rysuje wykres best/current/avg/worst dla jednego algorytmu.
    plt.figure(figsize=(8, 4.5))
    x = list(range(len(next(iter(data.values()), []))))
    for key in ["best", "current", "avg", "worst"]:
        if key in data:
            plt.plot(x, data[key], label=f"{key}")
    if optimal > 0:
        plt.axhline(optimal, color="green", linestyle="--", label=f"optimal {optimal}")
    plt.title(f"{instance} - {alg_name}")
    plt.xlabel("step")
    plt.ylabel("cost")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def plot_combined(instance: str, series_map: Dict[str, Dict[str, List[float]]], optimal: float, out_path: str) -> None:
    # Rysuje zestawienie best dla wszystkich algorytmów na jednym wykresie.
    plt.figure(figsize=(9, 5))
    colors = {"random": "tab:blue", "greedy": "tab:orange", "sa": "tab:purple", "ea": "tab:red"}
    for alg, data in series_map.items():
        if "best" not in data:
            continue
        # Specjalne traktowanie SA: rysujemy poziomą linię z finalnym best, żeby nie zdominować osi X.
        if alg == "sa" and data["best"]:
            final_best = data["best"][-1]
            plt.axhline(final_best, color=colors.get(alg, None), linestyle="--",
                        label=f"{alg} final best={final_best:.2f}")
        else:
            x = list(range(len(data["best"])))
            plt.plot(x, data["best"], label=alg, color=colors.get(alg, None))
    if optimal > 0:
        plt.axhline(optimal, color="green", linestyle="--", label=f"optimal {optimal}")
    plt.title(f"{instance} - porównanie best")
    plt.xlabel("step")
    plt.ylabel("cost")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def read_optimal(summary_path: str) -> Dict[str, float]:
    # Czyta optymalne wartości z summary.csv wygenerowanego przez program.
    opt = {}
    if not os.path.isfile(summary_path):
        return opt
    with open(summary_path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            inst = row.get("instance") or row.get("instancja") or row.get("instance".capitalize())
            if inst and row.get("optimal"):
                try:
                    opt[inst] = float(row["optimal"])
                except ValueError:
                    pass
    return opt


def read_summary(summary_path: str) -> Dict[str, Dict[str, float]]:
    # Czyta cały summary.csv jako słownik instancja -> pola liczbowe.
    data = {}
    if not os.path.isfile(summary_path):
        return data
    with open(summary_path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            inst = row.get("instance") or row.get("instancja") or row.get("instance".capitalize())
            if not inst:
                continue
            numeric_row = {}
            for k, v in row.items():
                try:
                    numeric_row[k] = float(v)
                except (TypeError, ValueError):
                    pass
            data[inst] = numeric_row
    return data


def plot_bar_best(instance: str, summary_row: Dict[str, float], out_path: str) -> None:
    # Rysuje wykres słupkowy z wartościami best dla wszystkich algorytmów.
    if not summary_row:
        return
    algs = ["random", "greedy", "sa", "ea"]
    values = []
    labels = []
    colors = ["tab:blue", "tab:orange", "tab:purple", "tab:red"]
    for alg in algs:
        key = f"{alg}_best"
        if key in summary_row:
            values.append(summary_row[key])
            labels.append(alg)
    plt.figure(figsize=(7, 4))
    bars = plt.bar(labels, values, color=colors[: len(values)])
    # Dodajemy etykiety liczby nad słupkami.
    for bar, val in zip(bars, values):
        plt.text(bar.get_x() + bar.get_width() / 2, bar.get_height(), f"{val:.0f}",
                 ha="center", va="bottom", fontsize=9)
    if "optimal" in summary_row and summary_row["optimal"] > 0:
        plt.axhline(summary_row["optimal"], color="green", linestyle="--", label=f"optimal {summary_row['optimal']:.2f}")
    plt.title(f"{instance} - best z wszystkich uruchomień")
    plt.ylabel("cost")
    if "optimal" in summary_row and summary_row["optimal"] > 0:
        plt.legend()
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def main():
    # Wczytujemy optima i dane z summary.csv tego katalogu logów (jeśli istnieje).
    summary_path = os.path.join(LOG_DIR, "summary.csv")
    optimal_map = read_optimal(summary_path)
    summary_map = read_summary(summary_path)
    # Iterujemy po podkatalogach (instancjach).
    for inst_dir in sorted(os.listdir(LOG_DIR)):
        full_inst_dir = os.path.join(LOG_DIR, inst_dir)
        if not os.path.isdir(full_inst_dir):
            continue
        if inst_dir.startswith("summary"):
            continue
        # Optymalna wartość.
        optimal_val = optimal_map.get(inst_dir, -1.0)
        # Zbieramy pliki run per alg.
        alg_patterns = {
            "random": os.path.join(full_inst_dir, "random_run_*.csv"),
            "greedy": os.path.join(full_inst_dir, "greedy_run_*.csv"),
            "sa": os.path.join(full_inst_dir, "sa_run_*.csv"),
            "ea": os.path.join(full_inst_dir, "ea_run_*.csv"),
        }
        series_for_combined: Dict[str, Dict[str, List[float]]] = {}
        # Wyjściowy katalog dla instancji.
        out_dir = os.path.join(OUTPUT_DIR, inst_dir)
        ensure_dir(out_dir)
        # Przechodzimy po algorytmach.
        for alg, pattern in alg_patterns.items():
            files = glob.glob(pattern)
            run_path = pick_run(files)
            if not run_path:
                continue
            data = read_series(run_path)
            series_for_combined[alg] = data
            # Wykres pojedynczy.
            single_path = os.path.join(out_dir, f"{alg}_single.png")
            plot_single(inst_dir, alg, data, optimal_val, single_path)
        # Wykres łączony.
        combo_path = os.path.join(out_dir, "combined_best.png")
        plot_combined(inst_dir, series_for_combined, optimal_val, combo_path)
        # Wykres słupkowy wartości best dla wszystkich algorytmów (na podstawie summary).
        bar_path = os.path.join(out_dir, "bar_best.png")
        plot_bar_best(inst_dir, summary_map.get(inst_dir, {}), bar_path)


if __name__ == "__main__":
    ensure_dir(OUTPUT_DIR)
    main()
