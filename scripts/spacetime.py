#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt

def main():
    df = pd.read_csv("results/space_time.csv")
    
    df = df[(df["time"] < 400) & (df["position"] < 400)]

    unique_lanes = sorted(df["lane"].unique())
    fig, axes = plt.subplots(1, len(unique_lanes), figsize=(6 * len(unique_lanes), 6), sharey=True)
    
    if len(unique_lanes) == 1: axes = [axes]

    for ax, lane in zip(axes, unique_lanes):
        lane_data = df[df["lane"] == lane]
        
        ax.scatter(lane_data["time"], lane_data["position"], c='black', s=1, marker='s')
        
        ax.set_title(f"{'Left' if lane == 0 else 'Right'} Lane")
        ax.set_xlabel("Time (steps)")
        ax.set_ylim(400, 0)

    axes[0].set_ylabel("Position (cells)")
    plt.tight_layout()
    plt.savefig("results/spacetime_diagram.png")
    print("Saved space-time diagram to spacetime_diagram.png")

if __name__ == "__main__":
    main()