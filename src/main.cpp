#include <stdlib.h>
#include <time.h>

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "CImg.h"
#include "Car.h"
#include "Lane.h"

using namespace cimg_library;

// TODO: for visualisation purposes

const int SCREEN_CELLS_X = 1000;
const int SCREEN_CELLS_Y = 1000;
const int CELL_PIXELS = 10;  // in pixels

const int DELTA = 1;                 // 1 second of model time per step
const int MAIN_LANE_LENGTH = 10000;  // in cells
const int VMAX = 60;

const double SLOW_TO_START_PROB = 0.5;  // q
const double RANDOMIZATION_PROB = 0.1;  // p

const int CAR_SPAWN_DISTANCE = 1;  // Minimum cells gap before spawning

// Global state
std::vector<std::unique_ptr<Lane>> g_lanes;
std::vector<std::unique_ptr<Car>> g_cars;
size_t next_car_id = 0;

void init_lanes() {
    int y_mid = SCREEN_CELLS_Y / 2;

    auto a_lane = std::make_unique<Lane>(Direction::EAST, MAIN_LANE_LENGTH, "a",
                                         0, y_mid);
    auto b_lane = std::make_unique<Lane>(Direction::EAST, MAIN_LANE_LENGTH, "b",
                                         0, y_mid + 1);

    g_lanes.push_back(std::move(a_lane));
    g_lanes.push_back(std::move(b_lane));
}

void draw(CImg<unsigned char>& img) {
    img.fill(0);

    for (auto& lane : g_lanes) {
        lane->draw(img);
    }
}

void spawn_cars() {
    // TODO: Random configuration on the road
    // TODO: Spawn car based on flow rate and choose lead and lag distance based
    // on aggression
    for (auto& lane : g_lanes) {
        // TODO: implement
    }
}

Lane* get_other_lane(Lane* lane) {
    for (auto& l : g_lanes) {
        if (l.get() != lane) return l.get();
    }
    return nullptr;
}

void switch_lane(Car* car, Lane* from_lane, Lane* to_lane) {
    auto& from_cars = from_lane->cars;

    std::remove_if(from_cars.begin(), from_cars.end(),
                   [car](const Car* c) { return c == car; });

    car->lane = to_lane;
    // TODO:
}

void apply_rules(Lane* lane, const Lane* prev_state,
                 const Lane* prev_state_other_lane) {
    for (size_t i = 0; i < lane->cars.size(); i++) {
        Car* car = lane->cars.at(i).get();

        // R1: slow-to-start
        if (car->speed == 0) {
            if (car->prev_front_dist == 0) {
                // With probability q, stay stopped
                if ((rand() % 100) < (SLOW_TO_START_PROB * 100)) {
                    continue;
                }
            }
        }

        // R2: Acceleration
        int accel_speed = std::min(car->speed + car->get_accel(), VMAX);

        // R3: Deceleration due to other cars
        // TODO: Handle no car ahead
        int front_dist = lane->front_gap(i);
        int decel_speed = std::min(accel_speed, front_dist);

        // R4: Randomization
        int rand_speed = decel_speed;
        if ((rand() % 100) < (RANDOMIZATION_PROB * 100))
            rand_speed = std::max(0, decel_speed - 1);

        // R5: Movement or Lane change
        Lane* other_lane = get_other_lane(lane);
        if (other_lane) {
            bool incentive = front_dist < min(accel_speed, VMAX);

            bool improvement =
                other_lane->target_front_gap(
                    other_lane->find_car_at_pos(car->pos)) > front_dist;

            bool safety = other_lane->target_back_gap(
                              other_lane->find_car_at_pos(car->pos)) > VMAX;

            if (incentive && improvement && safety) {
                switch_lane(car, lane, other_lane);
                // TODO: Do they move even in the new lane?
            }
        }

        car->speed = rand_speed;
    }
}

void remove_out_of_bounds_cars() {
    for (auto& lane : g_lanes) {
        lane->cars.erase(std::remove_if(lane->cars.begin(), lane->cars.end(),
                                        [](const std::unique_ptr<Car>& car) {
                                            return car->pos >=
                                                   car->lane->len_cels;
                                        }),
                         lane->cars.end());
    }
}

void sim_step(unsigned long mt) {
    spawn_cars();

    // state in t-1
    std::vector<Lane> prev_states;
    for (auto& lane : g_lanes) {
        prev_states.push_back(*lane);
    }

    // TODO: Ended here
    for (const auto& lane : g_lanes) {
        apply_rules(lane.get(), &prev_states[i],
                    &prev_states[(i + 1) % g_lanes.size()]);
    }

    remove_out_of_bounds_cars();
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
