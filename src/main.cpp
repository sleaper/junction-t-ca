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
    int rear_cell;

    int v = 0;
    int a = 1;
    int prev_front_dist = -1;

    bool aggressive = false;
};

struct StepMetrics {
    double model_time = 0.0;
    double density = 0.0;
    double flow = 0.0;
    double avg_speed = 0.0;         // cells per step
    double lane_change_rate = 0.0;  // lane changes per cell per step
};

class Statistics {
   public:
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
        out << "step,model_time,density,flow,avg_speed,lane_change_rate\n";
        for (size_t i = 0; i < samples_.size(); ++i) {
            const auto& s = samples_[i];
            out << i << ',' << s.model_time << ',' << s.density << ',' << s.flow
                << ',' << s.avg_speed << ',' << s.lane_change_rate << '\n';
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

int get_accel(int speed) {
    if (speed <= 12) {
        return 4;
    } else if (speed <= 22) {
        return 3;
    } else {
        return 2;
    }
}

int front_gap(const Car* car, const Lane* lane) {
    int gap = 0;
    int front_of_car = car->rear_cell + CAR_LEN;

    for (int i = 1; i < MAIN_LANE_LENGTH; i++) {
        int cell = (front_of_car + i) % MAIN_LANE_LENGTH;  // wrap around
        if (lane->occ.at(cell) != EMPTY_CELL && lane->occ.at(cell) != car->id) {
            return gap;
        }
        gap++;
    }

    return gap;
}

int back_gap(const Car* car, const Lane* lane) {
    int gap = 0;
    int back_of_car = car->rear_cell;

    for (int i = 1; i < MAIN_LANE_LENGTH; i++) {
        int cell = (back_of_car - i + MAIN_LANE_LENGTH) %
                   MAIN_LANE_LENGTH;  // wrap around
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

        while (spawned < n_cars) {
            rand_pos = rand() % (MAIN_LANE_LENGTH - CAR_LEN);

            // Check if position is free
            bool pos_free = true;
            for (int j = 0; j < CAR_LEN; j++) {
                if (lane->occ.at(rand_pos + j) != EMPTY_CELL) {
                    pos_free = false;
                    break;
                }
            }

            if (pos_free && (rand() % 100) < (density * 100)) {
                auto car = std::make_unique<Car>();
                car->id = static_cast<int>(next_car_id++);
                car->lane_id = lane->id;
                car->rear_cell = rand_pos;
                car->v = 0;

                for (int j = 0; j < CAR_LEN; j++) {
                    lane->occ.at(car->rear_cell + j) = car->id;
                }

                g_cars.push_back(std::move(car));
                spawned++;
                if (spawned >= n_cars) break;
            }
        }
    }
}

void sim_step(double mt) {
    if (mt == 0) spawn_cars(SPAWN_DENSITY);

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
        bool hesitate = false;
        int v_plan = car->v;

        int f_gap = front_gap(car, lane);

        // R1: slow-to-start
        if (car->v == 0 && car->prev_front_dist == 0 && f_gap > 0) {
            // With probability q, hesitate
            if ((rand() % 100) < (SLOW_TO_START_PROB * 100)) {
                hesitate = true;
            }
        }

        // R2: Acceleration
        if (!hesitate) v_plan = std::min(car->v + get_accel(car->v), VMAX);

        // R3: Deceleration due to other cars
        v_plan = std::min(v_plan, f_gap);

        // R4: Randomization
        if ((rand() % 100) < (BREAKING_PROB * 100)) {
            v_plan = std::max(v_plan - 1, 0);
        }

        // R5: Lane change
        Lane* other_lane = g_lanes.at(1 - car->lane_id).get();
        bool incentive = f_gap < std::min(car->v + get_accel(car->v), VMAX);
        bool improvement = front_gap(car, other_lane) > f_gap;
        bool safety = back_gap(car, other_lane) > VMAX;

        bool pos_free = true;
        for (size_t j = 0; j < CAR_LEN; j++) {
            int cell = (car->rear_cell + j) % MAIN_LANE_LENGTH;
            if (other_lane->occ.at(cell) != EMPTY_CELL) {
                pos_free = false;
                break;
            }
        }

        if (incentive && improvement && safety && pos_free) {
            if ((rand() % 100) < (LANE_CHANGE_PROB * 100)) {
                next_lane[i] = other_lane->id;
                lane_changes_this_step++;
            }
        }

        next_v[i] = v_plan;
        next_pos[i] = car->rear_cell + v_plan;
        car->prev_front_dist = f_gap;
    }

    // Prepare for next step
    for (auto& lane : g_lanes) {
        lane->clear_next();
    }

    // Commit updates to all cars
    for (size_t i = 0; i < g_cars.size(); i++) {
        Lane* target_lane = g_lanes.at(next_lane[i]).get();

        bool collision = false;
        for (int j = 0; j < CAR_LEN; j++) {
            int cell = (next_pos[i] + j) % MAIN_LANE_LENGTH;
            if (target_lane->next_occ.at(cell) != EMPTY_CELL) {
                collision = true;
                break;
            }
        }

        if (collision) {
            throw std::runtime_error("Collision detected for car " +
                                     std::to_string(g_cars[i]->id));
        }

        g_cars[i]->v = next_v[i];
        g_cars[i]->rear_cell = next_pos[i];
        g_cars[i]->lane_id = next_lane[i];

        Lane* lane = g_lanes.at(g_cars[i]->lane_id).get();
        for (int j = 0; j < CAR_LEN; j++) {
            int cell = (g_cars[i]->rear_cell + j) % MAIN_LANE_LENGTH;
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

    for (double step = 0; step < MAX_TIME_STEP; step += DELTA) {
        sim_step(step);

        if (visualize) {
            draw(grid);
            CImg<unsigned char> zoomed =
                grid.get_resize(WIN_W, WIN_H, -100, -100, 1);

            std::string iter_text = "Step: " + std::to_string(step);
            const unsigned char white[] = {255, 255, 255};
            zoomed.draw_text(WIN_W - 200, 10, iter_text.c_str(), white, 0, 1,
                             24);

            win.display(zoomed);

            if (win.is_closed()) break;
            if (win.resize()) {
                win.resize(WIN_W, WIN_H);
            }

            win.wait(1000);  // 1 second real-time delay for visualization
        }
    }

    try {
        const std::string stats_file = "stats.csv";
        g_stats.dump_csv(stats_file);
        std::cout << "Saved per-step statistics to " << stats_file << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Failed to write statistics: " << e.what() << "\n";
    }
    g_stats.print_summary();

    return 0;
}
