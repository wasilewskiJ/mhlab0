#!/usr/bin/env python3
# Skrypt rysuje przebiegi z logów CSV zapisanych przez program.
LOG_DIR = "logs_baseline"      # katalog z logami (per instancja podkatalog)
OUTPUT_DIR = "plots_baseline"  # katalog na wykresy
RUN_PICK_STRATEGY = "middle"   # "middle" wybiera środkowy run, można ustawić "first"/"last"/indeks int

import os
import glob
import math
import csv
from typing import List, Dict, Union

import matplotlib.pyplot as plt


def ensure_dir(path: str) -> None:
    # Tworzy katalog jeśli nie istnieje.
    os.makedirs(path, exist_ok=True)


def pick_run(files: List[str], strategy: str) -> str:
    # Wybiera plik run według strategii.
    if not files:
        return ""
    if strategy == "first":
        return sorted(files)[0]
    if strategy == "last":
        return sorted(files)[-1]
    if strategy == "middle":
        files_sorted = sorted(files)
        return files_sorted[len(files_sorted) // 2]
    if isinstance(strategy, int):
        files_sorted = sorted(files)
        idx = max(0, min(len(files_sorted) - 1, strategy))
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


def _limit_series(alg_name: str, data: Dict[str, List[float]]) -> Dict[str, List[float]]:
    # Nie ucinamy żadnych serii - pokazujemy wszystko.
    return data


def plot_single(instance: str, alg_name: str, data: Dict[str, List[float]], optimal: float, out_path: str) -> None:
    # Rysuje wykres best/current/avg/worst dla jednego algorytmu.
    data = _limit_series(alg_name, data)
    plt.figure(figsize=(10, 5.5), dpi=150)
    x = list(range(len(next(iter(data.values()), []))))
    best_val = None
    for key in ["best", "current", "avg", "worst"]:
        if key in data:
            if key == "best" and data[key]:
                best_val = min(data[key])
            label = key if best_val is None or key != "best" else f"{key} (min={best_val:.0f})"
            plt.plot(x, data[key], label=label)
    if optimal > 0:
        plt.axhline(optimal, color="green", linestyle="--", label=f"optimal {optimal}")
    title_map = {
        "ea": f"{instance} - Przebieg algorytmu ewolucyjnego (pojedyncze uruchomienie)",
        "sa": f"{instance} - Przebieg symulowanego wyżarzania (pojedyncze uruchomienie)",
        "greedy": f"{instance} - Przebieg algorytmu zachłannego (pojedyncze uruchomienie)",
        "random": f"{instance} - Przebieg algorytmu losowego (pojedyncze uruchomienie)",
    }
    plt.title(title_map.get(alg_name, f"{instance} - {alg_name} (pojedyncze uruchomienie)"))
    plt.xlabel("step" if alg_name != "ea" else "generation")
    plt.ylabel("cost")
    plt.xlim(left=0)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def plot_combined(instance: str, series_map: Dict[str, Dict[str, List[float]]], optimal: float, out_path: str) -> None:
    # Rysuje zestawienie best dla wszystkich algorytmów na jednym wykresie.
    plt.figure(figsize=(10, 5.5), dpi=150)
    colors = {"random": "tab:blue", "greedy": "tab:orange", "sa": "tab:purple", "ea": "tab:red"}
    
    # Znajdź maksymalną długość serii (pomijając SA, bo ma za dużo stepów).
    max_length = 0
    for alg, data in series_map.items():
        if alg == "sa":
            continue
        data_limited = _limit_series(alg, data)
        if "best" in data_limited and data_limited["best"]:
            max_length = max(max_length, len(data_limited["best"]))
    
    for alg, data in series_map.items():
        data_limited = _limit_series(alg, data)
        if "best" not in data_limited:
            continue
        best_series = data_limited["best"]
        if not best_series:
            continue
        min_best = min(best_series)
        label_text = f"{alg} (best={min_best:.0f})"
        
        # Specjalne traktowanie SA: rysujemy poziomą linię z finalnym best, żeby nie zdominować osi X.
        if alg == "sa":
            final_best = best_series[-1]
            plt.axhline(final_best, color=colors.get(alg, None), linestyle="--",
                        label=label_text)
        else:
            x = list(range(len(best_series)))
            # Rysuj ciągłą linię dla oryginalnej serii.
            plt.plot(x, best_series, label=label_text, color=colors.get(alg, None))
            
            # Jeśli seria jest krótsza niż max, dorysuj przerywaną linię na końcowej wartości.
            if len(best_series) < max_length:
                final_value = best_series[-1]
                x_extend = list(range(len(best_series) - 1, max_length))
                y_extend = [final_value] * len(x_extend)
                plt.plot(x_extend, y_extend, linestyle="--", color=colors.get(alg, None), alpha=0.6)
    
    if optimal > 0:
        plt.axhline(optimal, color="green", linestyle="--", label=f"optimal {optimal}")
    plt.title(f"{instance} - Przebieg najlepszych rozwiązań (pojedyncze uruchomienie)")
    plt.xlabel("step")
    plt.ylabel("cost")
    plt.xlim(0, max_length)
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
    plt.title(f"{instance} - najlepsze rozwiązania (spośród wszystkich uruchomień)")
    plt.ylabel("cost")
    if "optimal" in summary_row and summary_row["optimal"] > 0:
        plt.legend()
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def main(log_dir: str = LOG_DIR, out_dir: str = OUTPUT_DIR, run_pick_strategy: str = RUN_PICK_STRATEGY):
    # Wczytujemy optima i dane z summary.csv tego katalogu logów (jeśli istnieje).
    summary_path = os.path.join(log_dir, "summary.csv")
    optimal_map = read_optimal(summary_path)
    summary_map = read_summary(summary_path)
    # Iterujemy po podkatalogach (instancjach).
    for inst_dir in sorted(os.listdir(log_dir)):
        full_inst_dir = os.path.join(log_dir, inst_dir)
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
        inst_out_dir = os.path.join(out_dir, inst_dir)
        ensure_dir(inst_out_dir)
        # Przechodzimy po algorytmach.
        for alg, pattern in alg_patterns.items():
            files = glob.glob(pattern)
            run_path = pick_run(files, run_pick_strategy)
            if not run_path:
                continue
            data = read_series(run_path)
            series_for_combined[alg] = data
            # Wykres pojedynczy.
            single_path = os.path.join(inst_out_dir, f"{alg}_single.png")
            plot_single(inst_dir, alg, data, optimal_val, single_path)
        # Wykres łączony.
        combo_path = os.path.join(inst_out_dir, "combined_best.png")
        plot_combined(inst_dir, series_for_combined, optimal_val, combo_path)
        # Wykres słupkowy wartości best dla wszystkich algorytmów (na podstawie summary).
        bar_path = os.path.join(inst_out_dir, "bar_best.png")
        plot_bar_best(inst_dir, summary_map.get(inst_dir, {}), bar_path)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Rysuje wykresy z logów CSV wygenerowanych przez solver VRP.")
    parser.add_argument("--log-dir", default=LOG_DIR, help="Katalog z logami (domyślnie logs_baseline).")
    parser.add_argument("--out-dir", default=OUTPUT_DIR, help="Katalog wyjściowy na wykresy.")
    parser.add_argument("--run-pick", default=RUN_PICK_STRATEGY,
                        help="Strategia wyboru pojedynczego przebiegu: first/last/middle lub indeks.")
    args = parser.parse_args()

    strategy_value: Union[str, int]
    # Jeśli podano liczbę, zamień na int, inaczej użyj jako string.
    try:
        strategy_value = int(args.run_pick)
    except ValueError:
        strategy_value = args.run_pick

    ensure_dir(args.out_dir)
    main(log_dir=args.log_dir, out_dir=args.out_dir, run_pick_strategy=strategy_value)
