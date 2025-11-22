#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path


def main():
    csv_path = Path("final_statistics.csv")
    density, total_flow, lane_change, left_flow, right_flow = [], [], [], [], []
    
    with open(csv_path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            density.append(float(row["density"]))
            total_flow.append(float(row["total_flow"]))
            lane_change.append(float(row["lane_change_rate"]))
            left_flow.append(float(row["left_flow"]))
            right_flow.append(float(row["right_flow"]))
        
    fig, ax = plt.subplots(1, 2, figsize=(14, 6))
    ax[0].plot(density, total_flow, marker='o', label='Total Flow')
    ax[0].plot(density, left_flow, marker='s', label='Left Lane')
    ax[0].plot(density, right_flow, marker='^', label='Right Lane')
    ax[0].set_title("Fundamental Diagram: Flow vs Density")
    ax[0].set_xlabel("Density (cars/cell)")
    ax[0].set_ylabel("Flow (cars/time step)")
    ax[0].set_ylim(0, 0.45)
    ax[0].legend()
    ax[0].grid(True)
    ax[1].plot(density, lane_change, marker='o', color='orange')
    ax[1].set_title("Lane Change Rate vs Density")
    ax[1].set_xlabel("Density (cars/cell)")
    ax[1].set_ylabel("Lane Change Rate (changes/time step)")
    ax[1].set_ylim(0, 0.005)
    ax[1].grid(True)
    plt.tight_layout()
    plt.savefig("traffic_flow_lane_change.png", dpi=150, bbox_inches="tight")
    print("Saved plot to traffic_flow_lane_change.png")


if __name__ == "__main__":
    main()
