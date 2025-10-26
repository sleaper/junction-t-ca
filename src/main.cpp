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

const uint8_t INTERSECTION_COLOR[] = {255, 0, 0};

const int MAIN_LANE_LENGTH = 100;  // in cells
const int MINOR_LANE_LENGTH = 18;
const int SLIP_LANE_LENGTH = 17;

const int SCREEN_CELLS_X = 100;  // 750 / 100 = 7.5m per cell
const int SCREEN_CELLS_Y = 100;

constexpr double CELL_LENGTH_M = 7.5;
const int CELL_SIZE = 20;  // in pixels

const int DELTA = 1;  // 1 second of model time per step
const int VMAX = 9;

const double FLOW_RATE_MAJOR = 0.684;
const double FLOW_RATE_MINOR = 0.2;

const int CAR_SPAWN_DISTANCE = 1;  // Minimum cells gap before spawning

class IntersectionManager {
   public:
    std::vector<Lane*> lanes;

    IntersectionManager() {}

    void add_lane(Lane* lane) { lanes.push_back(lane); }

   private:
};

// Global state
std::vector<std::unique_ptr<Lane>> g_lanes;
std::vector<std::unique_ptr<Car>> g_cars;
IntersectionManager intersection_manager;
size_t next_car_id = 0;

void add_lane_to_system(std::unique_ptr<Lane> lane) {
    intersection_manager.add_lane(lane.get());
    g_lanes.push_back(std::move(lane));
}

void init_lanes() {
    int y_mid = SCREEN_CELLS_Y / 2;
    int x_mid = SCREEN_CELLS_X / 2;
    int y_last = SCREEN_CELLS_Y - 1;
    int x_last = SCREEN_CELLS_X - 1;
    int major_lane_offset = 3;
    int slip_lane_offset = 1;

    // Major lanes
    auto west =
        std::make_unique<Lane>(Direction::WEST, LaneType::THROUGH,
                               MAIN_LANE_LENGTH, "west", x_last, y_mid, false);

    auto slip = std::make_unique<Lane>(
        Direction::WEST, LaneType::TURN, SLIP_LANE_LENGTH, "slip",
        x_mid + SLIP_LANE_LENGTH + 1, y_mid + slip_lane_offset, false);

    auto east =
        std::make_unique<Lane>(Direction::EAST, LaneType::THROUGH,
                               MAIN_LANE_LENGTH, "east", 0, y_mid + 2, false);

    auto north = std::make_unique<Lane>(
        Direction::NORTH, LaneType::THROUGH, MINOR_LANE_LENGTH, "north", x_mid,
        y_mid + MINOR_LANE_LENGTH + major_lane_offset, true);

    auto south = std::make_unique<Lane>(Direction::SOUTH, LaneType::THROUGH,
                                        MINOR_LANE_LENGTH, "south", x_mid - 1,
                                        y_mid + major_lane_offset, true);

    add_lane_to_system(std::move(west));
    add_lane_to_system(std::move(slip));
    add_lane_to_system(std::move(east));
    add_lane_to_system(std::move(north));
    add_lane_to_system(std::move(south));
}

void draw(CImg<unsigned char>& img) {
    img.fill(0);

    for (auto& lane : g_lanes) {
        lane->draw(img);
    }

    // Draw intersection TODO: Maybe put into IntersectionManager ??
    for (int dx = 0; dx < 2; dx++) {
        for (int dy = 0; dy < 3; dy++) {
            int x = SCREEN_CELLS_X / 2 + dx - 1;
            int y = SCREEN_CELLS_Y / 2 + dy;
            img.draw_point(x, y, INTERSECTION_COLOR, 1);
        }
    }

    for (auto& car : g_cars) {
        car->draw(img);
    }
}

void spawn_cars() {
    for (auto& lane : g_lanes) {
        double flow_rate = (lane->type == LaneType::THROUGH) ? FLOW_RATE_MAJOR
                                                             : FLOW_RATE_MINOR;

        if (static_cast<double>(rand()) / RAND_MAX < flow_rate) {
            if (lane->find_car_at_pos(0) == nullptr) {
                float aggression = static_cast<float>(rand()) /
                                   RAND_MAX;  // TODO: come up with some
                                              // aggression distribution
                auto car = std::make_unique<Car>(lane->dir, aggression,
                                                 lane.get(), next_car_id++);
                lane->cars.push_back(car.get());
                g_cars.push_back(std::move(car));
            }
        }
    }
}

void apply_nash_rules(Lane* lane) {
    for (auto& car : lane->cars) {
        // R1: Acceleration
        if (car->speed < VMAX) {
            car->speed += 1;
        }

        // R2: Deceleration due to other cars
        int distance = lane->distance_to_next_car(car->id);
        if (distance != -1) car->speed = std::min(car->speed, distance);

        // R3: Randomization
        if (rand() % 2 == 1) car->speed = std::max(0, car->speed - 1);

        // R4: Movement
        car->pos += car->speed;
    }
}

void remove_out_of_bounds_cars() {
    g_cars.erase(
        std::remove_if(g_cars.begin(), g_cars.end(),
                       [](const std::unique_ptr<Car>& car) {
                           if (car->pos >= car->lane->len_cels) {
                               // Also remove from lane
                               auto& lane_cars = car->lane->cars;
                               lane_cars.erase(
                                   std::remove(lane_cars.begin(),
                                               lane_cars.end(), car.get()),
                                   lane_cars.end());
                               return true;
                           }
                           return false;
                       }),
        g_cars.end());
}

void sim_step() {
    spawn_cars();

    // for (auto& car : g_cars) {
    //     attempt_lane_change(car.get());
    // }

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
        sim_step();

        if (visualize) {
            draw(grid);
            CImg<unsigned char> zoomed =
                grid.get_resize(WIN_W, WIN_H, -100, -100, 1);
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
