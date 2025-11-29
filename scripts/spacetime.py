#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt

def main():
    df = pd.read_csv("space_time.csv")
    
    df = df[(df["time"] < 400) & (df["position"] < 400)]

    unique_lanes = sorted(df["lane"].unique())
    fig, axes = plt.subplots(1, len(unique_lanes), figsize=(6 * len(unique_lanes), 6), sharey=True)
    
    if len(unique_lanes) == 1: axes = [axes]

    for ax, lane in zip(axes, unique_lanes):
        lane_data = df[df["lane"] == lane]
        
        ax.scatter(lane_data["time"], lane_data["position"], c='black', s=1, marker='s')
        
        ax.set_title(f"Lane {lane}")
        ax.set_xlabel("Time")
        ax.set_ylim(400, 0)

    axes[0].set_ylabel("Position")
    plt.tight_layout()
    plt.savefig("spacetime_diagram.png")
    print("Saved space-time diagram to spacetime_diagram.png")

if __name__ == "__main__":
    main()