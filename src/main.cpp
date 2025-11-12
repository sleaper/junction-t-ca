#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <array>
#include <cstdint>
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

std::vector<std::unique_ptr<Lane>> g_lanes;
std::vector<std::unique_ptr<Car>> g_cars;
size_t next_car_id = 0;

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

    return std::min(gap, VMAX);  // No car ahead
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

    return std::min(gap, VMAX);  // No car behind
}

void spawn_cars() {
    Lane* lane = g_lanes.at(0).get();

    // Spawn pos is free
    for (int i = 0; i < CAR_LEN; i++) {
        if (lane->occ.at(i) != EMPTY_CELL) return;
    }

    // Check minimum gap ahead
    for (int i = CAR_LEN;
         i < CAR_LEN + CAR_SPAWN_DISTANCE && i < MAIN_LANE_LENGTH; i++) {
        if (lane->occ.at(i) != EMPTY_CELL) return;
    }

    auto car = std::make_unique<Car>();
    car->id = (int)next_car_id++;
    car->lane_id = lane->id;
    car->rear_cell = 0;
    car->v = 0;

    for (int i = 0; i < CAR_LEN; i++) {
        lane->occ.at(car->rear_cell + i) = car->id;
    }

    g_cars.push_back(std::move(car));
}

void sim_step(size_t mt) {
    if (mt == 0) spawn_cars();

    std::vector<int> next_v(g_cars.size());
    std::vector<int> next_pos(g_cars.size());
    std::vector<int> next_lane(g_cars.size());

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
        if ((rand() % 100) < (RANDOMIZATION_PROB * 100)) {
            std::cout << "Randomization for car " << car->id << "\n";
            v_plan = std::max(v_plan - 1, 0);
        }

        // R5: Lane change
        Lane* other_lane = g_lanes.at(1 - car->lane_id).get();
        bool incentive = f_gap < std::min(v_plan, VMAX);
        bool improvement = front_gap(car, other_lane) > f_gap;
        bool safety = back_gap(car, other_lane) > VMAX;

        bool pos_free = true;
        for (size_t i = 0; i < CAR_LEN; i++) {
            if (other_lane->occ.at(car->rear_cell + i) != EMPTY_CELL) {
                pos_free = false;
                break;
            }
        }

        std::cout << "Car " << car->id
                  << " lane change check: incentive=" << incentive
                  << ", improvement=" << improvement << ", safety=" << safety
                  << ", pos_free=" << pos_free << "\n";
        if (incentive && improvement && safety && pos_free) {
            std::cout << "Car " << car->id << " changing lane from "
                      << car->lane_id << " to " << other_lane->id << "\n";
            next_lane[i] = other_lane->id;
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
    for (size_t i = 0; i < g_cars.size();) {
        // Remove car beyond the lane
        if (next_pos[i] + CAR_LEN > MAIN_LANE_LENGTH) {
            g_cars.erase(g_cars.begin() + i);
            next_v.erase(next_v.begin() + i);
            next_pos.erase(next_pos.begin() + i);
            next_lane.erase(next_lane.begin() + i);
            continue;
        }

        g_cars[i]->v = next_v[i];
        g_cars[i]->rear_cell = next_pos[i];
        g_cars[i]->lane_id = next_lane[i];

        Lane* lane = g_lanes.at(g_cars[i]->lane_id).get();
        for (int j = 0; j < CAR_LEN; j++) {
            lane->next_occ.at(g_cars[i]->rear_cell + j) = g_cars[i]->id;
        }

        i++;
    }

    for (auto& lane : g_lanes) {
        lane->swap_buffers();
    }
}

int main() {
    init_lanes();

    srand(static_cast<unsigned int>(time(nullptr)));

    bool visualize = true;

    const int WIN_W = SCREEN_CELLS_X * CELL_PIXELS;
    const int WIN_H = SCREEN_CELLS_Y * CELL_PIXELS;
    CImg<unsigned char> grid(SCREEN_CELLS_X, SCREEN_CELLS_Y, 1, 3, 0);

    CImgDisplay win;

    if (visualize) win = CImgDisplay(WIN_W, WIN_H, "Simulation grid");

    for (unsigned long mt = 0; mt < 10000; mt += DELTA) {
        sim_step(mt);

        if (visualize) {
            draw(grid);
            CImg<unsigned char> zoomed =
                grid.get_resize(WIN_W, WIN_H, -100, -100, 1);

            std::string iter_text = "Step: " + std::to_string(mt);
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

        // TODO: Collect statistics
    }

    // TODO: Output statistics

    return 0;
}
