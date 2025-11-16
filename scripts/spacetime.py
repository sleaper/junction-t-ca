#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import numpy as np

def main():
    # Load data
    times, lanes, positions = [], [], []
    with open("space_time.csv") as f:
        reader = csv.DictReader(f)
        for row in reader:
            times.append(int(row["time"]))
            lanes.append(int(row["lane"]))
            positions.append(int(row["position"]))

    max_time = max(times)
    max_pos = max(positions)
    num_lanes = len(set(lanes))
    print(f"Time steps: {max_time + 1}, Positions: {max_pos + 1}, Lanes: {num_lanes}")

    display_time = min(800, max_time + 1)
    display_pos = min(800, max_pos + 1)

    # Create space-time matrices: rows = time, columns = position
    matrices = {}
    unique_lanes = sorted(set(lanes))
    for lane in unique_lanes:
        matrices[lane] = np.zeros((display_time, display_pos), dtype=int)

    # Fill matrices with vehicle positions
    for t, lane, pos in zip(times, lanes, positions):
        if t < display_time and pos < display_pos:
            matrices[lane][t, pos] = 1

    # Create figure with subplots for each lane
    fig, axes = plt.subplots(1, num_lanes, figsize=(8 * num_lanes, 8), sharey=True)
    lane_names = {0: "Left Lane", 1: "Right Lane"}

    for idx, lane in enumerate(unique_lanes):
        axes[idx].imshow(
            matrices[lane],
            cmap="binary",
            interpolation="none",
            aspect="auto",
            origin="upper",
        )
        axes[idx].set_xlabel("Position (cells)", fontsize=12)
        axes[idx].set_ylabel("Time Step", fontsize=12)
        axes[idx].set_title(lane_names.get(lane, f"Lane {lane}"), fontsize=14)

    plt.tight_layout()
    plt.savefig("spacetime_diagram.png", dpi=150, bbox_inches="tight")
    print("Saved space-time diagram to spacetime_diagram.png")

if __name__ == "__main__":
    main()