#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "CImg.h"
#include "Lane.h"
#include "config.h"

using namespace cimg_library;

struct Car {
    int id;
    int lane_id;
    int poss;
    int v = 0;

    bool aggressive = false;
};

struct StepMetrics {
    double model_time = 0.0;
    double density = 0.0;
    double flow = 0.0;
    double avg_speed = 0.0;         // cells per step
    double lane_change_rate = 0.0;  // lane changes per cell per step
    std::vector<std::vector<int>> lanes;
};

enum class LaneChangeMode { Symmetric, Asymmetric };

class Statistics {
   public:
    void save_final_statistics(const std::string& path,
                               std::vector<double>& density,
                               std::vector<double>& flow,
                               std::vector<double>& lane_change) const {
        std::ofstream out(path);
        if (!out.is_open()) {
            throw std::runtime_error("Failed to open " + path);
        }

        out << "density,flow,lane_change_rate\n";

        for (size_t i = 0; i < density.size(); i++) {
            out << density.at(i) << "," << flow.at(i) << ","
                << lane_change.at(i) << "\n";
        }
    }

    double get_average_flow() const {
        if (samples_.empty()) return 0.0;
        return flow_accum_ / static_cast<double>(samples_.size());
    }

    double get_average_lane_change_rate() const {
        if (samples_.empty()) return 0.0;
        return lane_change_accum_ / static_cast<double>(samples_.size());
    }

    void record_step(double model_time,
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
            sample.lanes.push_back(lane->occ);
        }

        double total_velocity = 0.0;
        for (const auto& car : cars) {
            total_velocity += car->v;
        }

        sample.density = total_cells > 0
                             ? occupied_cells / static_cast<double>(total_cells)
                             : 0.0;
        sample.flow = total_cells > 0
                          ? total_velocity / static_cast<double>(total_cells)
                          : 0.0;
        sample.avg_speed =
            cars.empty() ? 0.0
                         : total_velocity / static_cast<double>(cars.size());
        sample.lane_change_rate = total_cells > 0
                                      ? static_cast<double>(lane_changes) /
                                            static_cast<double>(total_cells)
                                      : 0.0;

        density_accum_ += sample.density;
        flow_accum_ += sample.flow;
        avg_speed_accum_ += sample.avg_speed;
        lane_change_accum_ += sample.lane_change_rate;

        samples_.push_back(sample);
    }

    void dump_csv(const std::string& path) const {
        std::ofstream out(path);
        if (!out.is_open()) {
            throw std::runtime_error("Failed to open " + path);
        }

        out << "model_time,density,flow,avg_speed,lane_change_rate\n";

        for (const auto& sample : samples_) {
            out << sample.model_time << "," << sample.density << ","
                << sample.flow << "," << sample.avg_speed << ","
                << sample.lane_change_rate << "\n";
        }
    }

    void dump_space_time(const std::string& path) const {
        std::ofstream out(path);
        if (!out.is_open()) {
            throw std::runtime_error("Failed to open " + path);
        }

        // Space-time diagram format: time,lane,position,car_id
        out << "time,lane,position,car_id\n";

        for (const auto& sample : samples_) {
            for (size_t lane_idx = 0; lane_idx < sample.lanes.size();
                 lane_idx++) {
                for (size_t pos = 0; pos < sample.lanes[lane_idx].size();
                     pos++) {
                    int car_id = sample.lanes[lane_idx][pos];
                    if (car_id != EMPTY_CELL) {
                        out << sample.model_time << "," << lane_idx << ","
                            << pos << "," << car_id << "\n";
                    }
                }
            }
        }
    }

    void print_summary() const {
        if (samples_.empty()) {
            std::cout << "No statistics collected.\n";
            return;
        }

        const double denom = static_cast<double>(samples_.size());
        std::cout << "Averaged over " << samples_.size() << " steps:\n";
        std::cout << "  Density: " << density_accum_ / denom << '\n';
        std::cout << "  Flow: " << flow_accum_ / denom << '\n';
        std::cout << "  Avg speed: " << avg_speed_accum_ / denom << '\n';
        std::cout << "  Lane-change rate: " << lane_change_accum_ / denom
                  << '\n';
    }

   private:
    std::vector<StepMetrics> samples_;
    double density_accum_ = 0.0;
    double flow_accum_ = 0.0;
    double avg_speed_accum_ = 0.0;
    double lane_change_accum_ = 0.0;
};

std::vector<std::unique_ptr<Lane>> g_lanes;
std::vector<std::unique_ptr<Car>> g_cars;
size_t next_car_id = 0;
Statistics g_stats;

void init_lanes() {
    int y_mid = SCREEN_CELLS_Y / 2;

    auto a_lane = std::make_unique<Lane>(0, 0, y_mid);
    auto b_lane = std::make_unique<Lane>(1, 0, y_mid + 1);

    g_lanes.push_back(std::move(a_lane));
    g_lanes.push_back(std::move(b_lane));
}

void draw(CImg<unsigned char>& img) {
    img.fill(0);

    for (auto& lane : g_lanes) {
        lane->draw(img);
    }
}

int front_gap(const Car* car, const Lane* lane) {
    int gap = 0;
    int car_pos = car->poss;

    for (int i = 1; i <= MAIN_LANE_LENGTH; i++) {
        int cell = (car_pos + i) % MAIN_LANE_LENGTH;  // wrap around
        if (lane->occ.at(cell) != EMPTY_CELL && lane->occ.at(cell) != car->id) {
            return gap;
        }
        gap++;
    }

    return gap;
}

int back_gap(const Car* car, const Lane* lane) {
    int gap = 0;
    int car_pos = car->poss;

    for (int i = 1; i <= MAIN_LANE_LENGTH; i++) {
        int cell =
            (car_pos - i + MAIN_LANE_LENGTH) % MAIN_LANE_LENGTH;  // wrap around
        if (lane->occ.at(cell) != EMPTY_CELL && lane->occ.at(cell) != car->id) {
            return gap;
        }
        gap++;
    }

    return gap;
}

void spawn_cars(double density) {
    // Spawn n cars in random configuration
    int n_cars = static_cast<int>(density * MAIN_LANE_LENGTH / CAR_LEN);

    for (size_t i = 0; i < g_lanes.size(); i++) {
        Lane* lane = g_lanes.at(i).get();
        int spawned = 0;

        int rand_pos = rand() % (MAIN_LANE_LENGTH - CAR_LEN);

        int attempts = 0;
        const int max_attempts = MAIN_LANE_LENGTH * 10;
        while (spawned < n_cars && attempts < max_attempts) {
            rand_pos = rand() % (MAIN_LANE_LENGTH - CAR_LEN);
            attempts++;

            // Check if position is free
            if (lane->occ.at(rand_pos) != EMPTY_CELL) continue;

            auto car = std::make_unique<Car>();
            car->id = static_cast<int>(next_car_id++);
            car->lane_id = lane->id;
            car->poss = rand_pos;
            car->v = (rand() % (VMAX + 1));

            for (int j = 0; j < CAR_LEN; j++) {
                lane->occ.at(car->poss + j) = car->id;
            }

            g_cars.push_back(std::move(car));
            spawned++;
        }
    }
}

void sim_step(double mt, double density) {
    if (mt == 0) spawn_cars(density);

    std::vector<int> next_v(g_cars.size());
    std::vector<int> next_pos(g_cars.size());
    std::vector<int> next_lane(g_cars.size());
    size_t lane_changes_this_step = 0;

    // Initialize next_lane
    for (size_t i = 0; i < g_cars.size(); i++) {
        next_lane[i] = g_cars[i]->lane_id;
    }

    for (size_t i = 0; i < g_cars.size(); i++) {
        Car* car = g_cars[i].get();
        Lane* lane = g_lanes.at(car->lane_id).get();
        int v_plan = car->v;

        // Lane change
        int look_ahead = v_plan + 1;
        int look_other_ahead = look_ahead;

        Lane* other_lane = g_lanes.at(1 - car->lane_id).get();
        // T1 somebody in my way
        bool incentive =
            (car->lane_id == LaneType::Right || MODE == MODE_TYPE::SYMMETRIC)
                ? front_gap(car, lane) < look_ahead
                : true;

        // T2 is other lane better?
        bool improvement = front_gap(car, other_lane) > look_other_ahead;

        // T3 is it safe?
        bool safety = back_gap(car, other_lane) > LOOK_OTHER_AHEAD;

        if (incentive && improvement && safety) {
            if (((double)rand() / RAND_MAX) < (LANE_CHANGE_PROB)) {
                next_lane[i] = other_lane->id;
                lane = other_lane;
                lane_changes_this_step++;
            }
        }

        // Movement in lane
        // S1: Acceleration
        v_plan = std::min(car->v + 1, VMAX);

        // S2: Deceleration due to other cars
        v_plan = std::min(v_plan, front_gap(car, lane));

        // S3: Randomization
        if (v_plan > 0 && ((double)rand() / RAND_MAX) < BREAKING_PROB) {
            v_plan = v_plan - 1;
        }

        next_v[i] = v_plan;
        next_pos[i] = car->poss + v_plan;
    }

    // Prepare for next step
    for (auto& lane : g_lanes) {
        lane->clear_next();
    }

    // Commit updates to all cars
    for (size_t i = 0; i < g_cars.size(); i++) {
        Lane* target_lane = g_lanes.at(next_lane[i]).get();

        g_cars[i]->v = next_v[i];
        g_cars[i]->poss = next_pos[i];
        g_cars[i]->lane_id = next_lane[i];

        Lane* lane = g_lanes.at(g_cars[i]->lane_id).get();
        for (int j = 0; j < CAR_LEN; j++) {
            int cell = (g_cars[i]->poss + j) % MAIN_LANE_LENGTH;
            lane->next_occ.at(cell) = g_cars[i]->id;
        }
    }

    for (auto& lane : g_lanes) {
        lane->swap_buffers();
    }

    g_stats.record_step(static_cast<double>(mt), g_cars, g_lanes,
                        lane_changes_this_step);
}

int main() {
    init_lanes();

    srand(static_cast<unsigned int>(time(nullptr)));

    bool visualize = false;

    const int WIN_W = SCREEN_CELLS_X * CELL_PIXELS;
    const int WIN_H = SCREEN_CELLS_Y * CELL_PIXELS;
    CImg<unsigned char> grid(SCREEN_CELLS_X, SCREEN_CELLS_Y, 1, 3, 0);

    CImgDisplay win;

    if (visualize) win = CImgDisplay(WIN_W, WIN_H, "Simulation grid");

    std::vector<double> flow;
    std::vector<double> lane_change;
    std::vector<double> densities;

    for (double density = 0.01; density <= 0.5; density += 0.01) {
        // Reset simulation state
        g_lanes.clear();
        g_cars.clear();
        next_car_id = 0;
        g_stats = Statistics();

        init_lanes();

        for (double step = 0; step < MAX_TIME_STEP; step += DELTA) {
            sim_step(step, density);

            if (visualize) {
                draw(grid);
                CImg<unsigned char> zoomed =
                    grid.get_resize(WIN_W, WIN_H, -100, -100, 1);

                std::string iter_text = "Step: " + std::to_string(step);
                const unsigned char white[] = {255, 255, 255};
                zoomed.draw_text(WIN_W - 200, 10, iter_text.c_str(), white, 0,
                                 1, 24);

                win.display(zoomed);

                if (win.is_closed()) break;
                if (win.resize()) {
                    win.resize(WIN_W, WIN_H);
                }

                win.wait(1000);  // 1 second real-time delay for visualization
            }
        }

        densities.push_back(density);
        flow.push_back(g_stats.get_average_flow());
        lane_change.push_back(g_stats.get_average_lane_change_rate());
        g_stats.print_summary();
    }

    try {
        g_stats.save_final_statistics("final_statistics.csv", densities, flow,
                                      lane_change);
    } catch (const std::exception& e) {
        std::cerr << "Error dumping statistics: " << e.what() << '\n';
    }

    return 0;
}
