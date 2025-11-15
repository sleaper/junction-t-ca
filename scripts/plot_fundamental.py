#!/usr/bin/env python3
import argparse
import csv
from pathlib import Path
import matplotlib.pyplot as plt


def load_data(csv_path, warmup):
    """Load density and flow data from CSV, skipping warmup rows."""
    with open(csv_path) as f:
        reader = csv.DictReader(f)
        data = [(float(row["density"]), float(row["flow"])) 
                for i, row in enumerate(reader) if i >= warmup]
    
    if not data:
        raise ValueError(f"No data after warmup in {csv_path}")
    
    densities, flows = zip(*data)
    return list(densities), list(flows)


def main():
    parser = argparse.ArgumentParser(description="Plot fundamental diagram from simulation data")
    parser.add_argument("stats_files", nargs="+", type=Path, help="CSV files from simulator")
    parser.add_argument("--labels", nargs="*", help="Labels for each CSV (default: filename)")
    parser.add_argument("--warmup", type=int, default=0, help="Steps to skip (default: 0)")
    parser.add_argument("--output", type=Path, default=Path("fundamental_diagram.png"), 
                        help="Output image (default: fundamental_diagram.png)")
    parser.add_argument("--show", action="store_true", help="Display plot interactively")
    parser.add_argument("--per-step", action="store_true", help="Show individual step data")
    args = parser.parse_args()

    if args.labels and len(args.labels) != len(args.stats_files):
        parser.error("Number of labels must match number of CSV files")

    # Load and process data
    series = []
    for i, csv_path in enumerate(args.stats_files):
        densities, flows = load_data(csv_path, args.warmup)
        series.append({
            "density": sum(densities) / len(densities),
            "flow": sum(flows) / len(flows),
            "densities": densities,
            "flows": flows,
            "label": args.labels[i] if args.labels else csv_path.stem,
        })

    series.sort(key=lambda x: x["density"])

    # Plot
    fig, ax = plt.subplots(figsize=(7, 5))

    if args.per_step:
        for s in series:
            ax.scatter(s["densities"], s["flows"], s=10, alpha=0.25, label=f"{s['label']} steps")

    mean_densities = [s["density"] for s in series]
    mean_flows = [s["flow"] for s in series]
    
    ax.plot(mean_densities, mean_flows, "-o", linewidth=2, markersize=6, 
            color="tab:orange", label="Mean per run")

    for s in series:
        ax.annotate(s["label"], (s["density"], s["flow"]), 
                   textcoords="offset points", xytext=(0, 10), ha="center")

    ax.set_xlabel("Density (fraction of occupied cells)")
    ax.set_ylabel("Flow (cars per cell per step)")
    ax.set_title("Fundamental Diagram")
    ax.grid(True, alpha=0.3, linestyle="--")
    ax.set_xlim(0, 1.05 * max(mean_densities + [0.1]))
    ax.legend()

    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Saved plot to {args.output}")

    if args.show:
        plt.show()


if __name__ == "__main__":
    main()
