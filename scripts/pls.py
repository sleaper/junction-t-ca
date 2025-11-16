#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path


def main():
    csv_path = Path("final_statistics.csv")
    density, flow, lane_change = [], [], []
    
    with open(csv_path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            density.append(float(row["density"]))
            flow.append(float(row["flow"]))
            lane_change.append(float(row["lane_change_rate"]))
        
    fig, ax = plt.subplots(1, 2, figsize=(14, 6))
    ax[0].plot(density, flow, marker='o')
    ax[0].set_title("Fundamental Diagram: Flow vs Density")
    ax[0].set_xlabel("Density (cars/cell)")
    ax[0].set_ylabel("Flow (cars/time step)")
    ax[0].set_ylim(0, 1)
    ax[0].grid(True)
    ax[1].plot(density, lane_change, marker='o', color='orange')
    ax[1].set_title("Lane Change Rate vs Density")
    ax[1].set_xlabel("Density (cars/cell)")
    ax[1].set_ylabel("Lane Change Rate (changes/time step)")
    ax[1].grid(True)
    plt.tight_layout()
    plt.savefig("traffic_flow_lane_change.png", dpi=150, bbox_inches="tight")
    print("Saved plot to traffic_flow_lane_change.png")


if __name__ == "__main__":
    main()
