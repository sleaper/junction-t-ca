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

const int CELL_SIZE = 10;  // in pixels

const int DELTA = 1;                // 1 second of model time per step
const int MAIN_LANE_LENGTH = 1000;  // in cells
const int VMAX = 60;

const double SLOW_TO_START_PROB = 0.5;  // q
const double RANDOMIZATION_PROB = 0.1;  // p

const int CAR_SPAWN_DISTANCE = 1;  // Minimum cells gap before spawning

// Global state
std::vector<std::unique_ptr<Lane>> g_lanes;
std::vector<std::unique_ptr<Car>> g_cars;  // Keep cars alive
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

    for (auto& car : g_cars) {
        car->draw(img);
    }
}

void spawn_one_car(Lane* lane) {
    // TODO: Placeholders for now
    int lead_distance = rand() % 3;
    int lag_distance = rand() % 3;

    auto car = std::make_unique<Car>(lane->dir, lane, next_car_id++);

    lane->cars.insert(lane->cars.begin(), car.get());
    g_cars.push_back(std::move(car));  // Keep the car alive
}

// TODO: Broken
void spawn_cars() {
    // TODO: Spawn car based on flow rate and choose lead and lag distance based
    // on aggression
    for (auto& lane : g_lanes) {
        // TODO: implement
    }
}

void apply_nash_rules(Lane* lane) {
    for (size_t i = 0; i < lane->cars.size(); i++) {
        Car* car = lane->cars.at(i);

        // R0: slow-to-start

        // R1: Acceleration
        if (car->speed < VMAX) {
            car->speed += 1;
        }

        // R2: Deceleration due to other cars
        int distance = lane->distance_to_next_car(i);
        if (distance != -1) car->speed = std::min(car->speed, distance);

        // R3: Randomization
        if (rand() % 2 == 1) car->speed = std::max(0, car->speed - 1);

        // R4: Lane change
        // R4a: incentive + improvement
        // R4b: safety check
        // R4c: opportunity

        // R5: Movement
        car->pos += car->speed;
    }
}

void remove_out_of_bounds_cars() {
    for (auto& lane : g_lanes) {
        lane->cars.erase(
            std::remove_if(
                lane->cars.begin(), lane->cars.end(),
                [](const Car* car) { return car->pos >= car->lane->len_cels; }),
            lane->cars.end());
    }
    g_cars.erase(std::remove_if(g_cars.begin(), g_cars.end(),
                                [](const std::unique_ptr<Car>& car) {
                                    return car->pos >= car->lane->len_cels;
                                }),
                 g_cars.end());
}

void sim_step(unsigned long mt) {
    // spawn_cars();
    if (mt == 0) spawn_one_car(g_lanes.at(0).get());

    for (auto& lane : g_lanes) {
        apply_nash_rules(lane.get());
    }

    remove_out_of_bounds_cars();
}

int main() {
    init_lanes();

    srand(static_cast<unsigned int>(time(nullptr)));

    bool visualize = true;

    const int WIN_W = SCREEN_CELLS_X * CELL_SIZE;
    const int WIN_H = SCREEN_CELLS_Y * CELL_SIZE;
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
