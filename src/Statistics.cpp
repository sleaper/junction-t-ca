#include "Statistics.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "config.h"

void Statistics::record_step(double model_time,
                             const std::vector<std::unique_ptr<Car>>& cars,
                             const std::vector<std::unique_ptr<Lane>>& lanes,
                             size_t lane_changes) {
    StepMetrics sample;
    sample.model_time = model_time;

    const size_t total_cells =
        lanes.size() * static_cast<size_t>(MAIN_LANE_LENGTH);
    double occupied_cells = 0.0;
    for (const auto& lane : lanes) {
        occupied_cells +=
            std::count_if(lane->occ.begin(), lane->occ.end(),
                          [](int id) { return id != EMPTY_CELL; });
        sample.lanes.push_back(lane->occ);  // Save lane occupancy
    }

    double total_left_velocity = 0.0;
    double total_right_velocity = 0.0;
    for (const auto& car : cars) {
        if (car->lane_id == LaneType::Left) {
            total_left_velocity += car->v;
        } else {
            total_right_velocity += car->v;
        }
    }

    const double lane_cells = static_cast<double>(MAIN_LANE_LENGTH);
    sample.left_flow = total_left_velocity / lane_cells;
    sample.right_flow = total_right_velocity / lane_cells;
    sample.density = occupied_cells / static_cast<double>(total_cells);
    sample.flow = (total_left_velocity + total_right_velocity) /
                  static_cast<double>(total_cells);
    sample.lane_change_rate =
        static_cast<double>(lane_changes) / static_cast<double>(total_cells);

    density_accum_ += sample.density;
    flow_accum_ += sample.flow;
    lane_change_accum_ += sample.lane_change_rate;
    left_flow_accum_ += sample.left_flow;
    right_flow_accum_ += sample.right_flow;

    samples_.push_back(sample);
}

void Statistics::dump_space_time(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open " + path);
    }

    // Space-time diagram format: time,lane,position,car_id
    out << "time,lane,position,car_id\n";

    for (const auto& sample : samples_) {
        for (size_t lane_idx = 0; lane_idx < sample.lanes.size(); lane_idx++) {
            for (size_t pos = 0; pos < sample.lanes[lane_idx].size(); pos++) {
                int car_id = sample.lanes[lane_idx][pos];
                if (car_id != EMPTY_CELL) {
                    out << sample.model_time << "," << lane_idx << "," << pos
                        << "," << car_id << "\n";
                }
            }
        }
    }
}

void Statistics::print_csv() const {
    if (samples_.empty()) {
        std::cerr << "No samples recorded.\n";
        return;
    }

    std::cout << "density,flow,lane_change_rate,left_flow,right_flow\n";
    std::cout << density_accum_ / samples_.size() << ","
              << flow_accum_ / samples_.size() << ","
              << lane_change_accum_ / samples_.size() << ","
              << left_flow_accum_ / samples_.size() << ","
              << right_flow_accum_ / samples_.size() << "\n";
}

void Statistics::print_summary() const {
    if (samples_.empty()) {
        std::cerr << "No samples recorded.\n";
        return;
    }

    std::cout << "=== Simulation Summary ===\n";
    std::cout << "Average Density: " << density_accum_ / samples_.size()
              << "\n";
    std::cout << "Average Flow: " << flow_accum_ / samples_.size() << "\n";
    std::cout << "Average Lane Change Rate: "
              << lane_change_accum_ / samples_.size() << "\n";
    std::cout << "Average Left Lane Flow: "
              << left_flow_accum_ / samples_.size() << "\n";
    std::cout << "Average Right Lane Flow: "
              << right_flow_accum_ / samples_.size() << "\n";
}
