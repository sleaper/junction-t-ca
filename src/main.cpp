#include <stdlib.h>
#include <time.h>

#include <array>
#include <cstdint>
#include <vector>

#include "CImg.h"

using namespace cimg_library;

const uint8_t CAR_COLOR[] = {0, 0, 255};
const uint8_t NORTH_ROAD[] = {255, 255, 255};
const uint8_t MIDDLE_ROAD[] = {150, 150, 150};
const uint8_t SOUTH_ROAD[] = {200, 200, 200};
const uint8_t WEST_ROAD[] = {100, 100, 100};
const uint8_t EAST_ROAD[] = {0, 255, 0};

const int CELLS_X = 100;  // 750 / 100 = 7.5m per cell
constexpr double CELL_LENGTH_M = 7.5;

const int CELLS_Y = 100;
const int CELL_SIZE = 10;

const int DELTA = 1;  // 1 second of model time per step
const int VMAX = 9;

const double FLOW_RATE = 0.684;    // cars per second (2464 cars/hour)
const int CAR_SPAWN_DISTANCE = 1;  // Minimum cells gap before spawning

typedef struct Car {
    int speed = 0;
    size_t x = 0;
} Car;

std::vector<Car> cars;

int distance_to_next_car(size_t car_index) {
    if (car_index >= cars.size() - 1) {  // last car
        // No car ahead
        return -1;
    } else {
        return (int)(cars.at(car_index + 1).x - cars.at(car_index).x - 1);
    }
}

void draw(CImg<unsigned char>& img) {
    img.fill(0);
    int y_mid = img.height() / 2;
    int y_last = img.height() - 1;
    int x_last = img.width() - 1;

    img.draw_line(0, y_mid, x_last, y_mid, NORTH_ROAD, 1);
    img.draw_line(0, y_mid + 1, x_last, y_mid + 1, MIDDLE_ROAD, 1);
    img.draw_line(0, y_mid + 2, x_last, y_mid + 2, SOUTH_ROAD, 1);
    img.draw_line(x_last / 2, y_mid, x_last / 2, y_last, WEST_ROAD, 1);
    img.draw_line((x_last / 2) - 1, y_mid, (x_last / 2) - 1, y_last, EAST_ROAD,
                  1);

    for (auto& car : cars) {
        if (car.x >= CELLS_X) continue;  // Out of bounds
        img.draw_point((int)car.x, y_mid, CAR_COLOR, 1);
    }
}

void sim_step() {
    if ((cars.empty() || cars.front().x != 0) &&
        (double)rand() / RAND_MAX < FLOW_RATE) {
        cars.insert(cars.begin(), {(rand() % VMAX) + 1, 0});
    }

    // R1: Acceleration
    for (auto& car : cars) {
        if (car.speed < VMAX) car.speed += 1;
    }

    // R2: Deceleration
    for (size_t i = 0; i < cars.size(); i++) {
        int distance = distance_to_next_car(i);
        if (distance == -1) continue;  // No car ahead

        cars[i].speed = std::min(cars[i].speed, distance - 1);
    }

    // R3: Randomization TODO: Pick right probability
    for (auto& car : cars) {
        if (rand() % 2 == 0) car.speed = std::max(0, car.speed - 1);
    }

    // R4: Car movement
    for (auto& car : cars) {
        car.x += car.speed;
    }

    // Remove cars that are out of bounds
    cars.erase(std::remove_if(cars.begin(), cars.end(),
                              [](const Car& car) { return car.x >= CELLS_X; }),
               cars.end());
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    bool visualize = true;

    const int WIN_W = CELLS_X * CELL_SIZE;
    const int WIN_H = CELLS_Y * CELL_SIZE;
    CImg<unsigned char> grid(CELLS_X, CELLS_Y, 1, 3, 0);

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
