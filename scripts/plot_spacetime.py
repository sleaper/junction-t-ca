#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path


def main():
    # Load data
    csv_path = Path("spacetime.csv")
    times, lanes, positions, car_ids = [], [], [], []
    
    with open(csv_path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            times.append(float(row["time"]))
            lanes.append(int(row["lane"]))
            positions.append(int(row["position"]))
            car_ids.append(int(row["car_id"]))
    
    # Get unique time steps, lanes, and position range
    unique_times = sorted(set(times))
    unique_lanes = sorted(set(lanes))
    max_position = max(positions)
    
    # Create subplots for each lane
    fig, axes = plt.subplots(len(unique_lanes), 1, 
                            figsize=(12, 6 * len(unique_lanes)),
                            sharex=True)
    
    if len(unique_lanes) == 1:
        axes = [axes]
    
    # Create space-time matrices for each lane
    for lane_id in unique_lanes:
        # Initialize matrix: rows = time steps, columns = positions
        spacetime_matrix = np.zeros((len(unique_times), max_position + 1))
        
        # Fill matrix: 1 if car is present, 0 if empty
        for t, p, l in zip(times, positions, lanes):
            if l == lane_id:
                time_idx = unique_times.index(t)
                spacetime_matrix[time_idx, p] = 1
        
        # Display as image (black and white)
        axes[lane_id].imshow(spacetime_matrix, cmap="Greys", 
                            interpolation="none", aspect="auto",
                            extent=[0, max_position, len(unique_times), 0])
        axes[lane_id].set_ylabel(f"Time Step\n(Lane {lane_id})")
        axes[lane_id].set_xlabel("Position (cells)")
    
    fig.suptitle("Space-Time Diagram: Traffic Flow")
    
    plt.tight_layout()
    plt.savefig("spacetime_diagram.png", dpi=150, bbox_inches="tight")
    print("Saved space-time diagram to spacetime_diagram.png")


if __name__ == "__main__":
    main()
