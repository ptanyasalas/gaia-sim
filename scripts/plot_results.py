#!/usr/bin/env python3
"""Generate evolution plots from the CSV produced by the C++ simulator.

Usage:
    python3 scripts/plot_results.py evolution_stats_42.csv
    python3 scripts/plot_results.py evolution_stats_42.csv --output-dir plots
"""

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def read_csv(path: Path):
    with path.open("r", newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    if not rows:
        raise ValueError(f"El CSV és buit: {path}")

    numeric = {}
    for key in rows[0]:
        values = []
        for row in rows:
            value = row[key]
            try:
                values.append(float(value))
            except ValueError:
                values.append(value)
        numeric[key] = values
    return numeric


def plot_population(data, output_dir: Path, stem: str):
    fig, ax = plt.subplots(figsize=(11, 5.5))
    ax.plot(data["Tick"], data["Herbivores"], label="Herbívors")
    ax.plot(data["Tick"], data["Predators"], label="Depredadors")
    ax.set_title("Evolució de les poblacions")
    ax.set_xlabel("Tick")
    ax.set_ylabel("Individus")
    ax.grid(True, alpha=0.25)
    ax.legend()
    fig.tight_layout()
    fig.savefig(output_dir / f"{stem}_populations.png", dpi=160)
    plt.close(fig)


def plot_traits(data, output_dir: Path, stem: str, prefix: str, label: str):
    traits = ["Attractiveness", "Vision", "Speed", "Size", "Metabolism"]
    labels = ["Atractiu", "Visió", "Velocitat", "Mida", "Metabolisme"]

    fig, axes = plt.subplots(3, 2, figsize=(12, 10), sharex=True)
    axes = axes.ravel()

    for ax, trait, trait_label in zip(axes, traits, labels):
        column = f"{prefix}Avg{trait}"
        ax.plot(data["Tick"], data[column])
        ax.set_title(trait_label)
        ax.set_ylabel("Mitjana")
        ax.grid(True, alpha=0.25)

    axes[-1].axis("off")
    axes[-2].set_xlabel("Tick")
    axes[-1].set_xlabel("")
    fig.suptitle(f"Evolució dels traits — {label}", fontsize=14)
    fig.tight_layout()
    fig.savefig(output_dir / f"{stem}_{prefix.lower()}_traits.png", dpi=160)
    plt.close(fig)


def plot_trait_comparison(data, output_dir: Path, stem: str):
    traits = ["Attractiveness", "Vision", "Speed", "Size", "Metabolism"]
    labels = ["Atractiu", "Visió", "Velocitat", "Mida", "Metabolisme"]

    fig, axes = plt.subplots(3, 2, figsize=(12, 10), sharex=True)
    axes = axes.ravel()
    for ax, trait, trait_label in zip(axes, traits, labels):
        ax.plot(data["Tick"], data[f"HerbivoreAvg{trait}"], label="Herbívors")
        ax.plot(data["Tick"], data[f"PredatorAvg{trait}"], label="Depredadors")
        ax.set_title(trait_label)
        ax.set_ylabel("Mitjana")
        ax.grid(True, alpha=0.25)
        ax.legend()
    axes[-1].axis("off")
    axes[-2].set_xlabel("Tick")
    fig.suptitle("Comparació de traits entre espècies", fontsize=14)
    fig.tight_layout()
    fig.savefig(output_dir / f"{stem}_trait_comparison.png", dpi=160)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description="Genera gràfiques del CSV de la simulació evolutiva.")
    parser.add_argument("csv", type=Path, help="CSV generat per evolution_sim")
    parser.add_argument("--output-dir", type=Path, default=Path("plots"), help="Carpeta de sortida")
    args = parser.parse_args()

    data = read_csv(args.csv)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    stem = args.csv.stem

    required = {
        "Tick", "Herbivores", "Predators",
        "HerbivoreAvgAttractiveness", "HerbivoreAvgVision", "HerbivoreAvgSpeed",
        "HerbivoreAvgSize", "HerbivoreAvgMetabolism",
        "PredatorAvgAttractiveness", "PredatorAvgVision", "PredatorAvgSpeed",
        "PredatorAvgSize", "PredatorAvgMetabolism",
    }
    missing = required.difference(data)
    if missing:
        raise ValueError("Falten columnes al CSV: " + ", ".join(sorted(missing)))

    plot_population(data, args.output_dir, stem)
    plot_traits(data, args.output_dir, stem, "Herbivore", "Herbívors")
    plot_traits(data, args.output_dir, stem, "Predator", "Depredadors")
    plot_trait_comparison(data, args.output_dir, stem)

    print(f"Gràfiques generades a: {args.output_dir.resolve()}")


if __name__ == "__main__":
    main()
