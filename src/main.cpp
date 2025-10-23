#include <stdlib.h>
#include <time.h>

#include <cstdint>
#include <vector>

#include "CImg.h"

using namespace cimg_library;

const unsigned char NORTH_ROAD[] = {255, 255, 255};
const unsigned char CAR_COLOR[] = {0, 0, 255};
// const unsigned char MIDDLE_ROAD[] = {150, 150, 150};
// const unsigned char SOUTH_ROAD[] = {200, 200, 200};
// const unsigned char WEST_ROAD[] = {100, 100, 100};

const int CELLS_X = 100;
const int CELLS_Y = 100;
const int CELL_SIZE = 10;

const int STEP = 1;
const int VMAX = 9;

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
        return cars[car_index + 1].x - cars[car_index].x;
    }
}

void draw_intersection(CImg<unsigned char>& img) {
    img.fill(0);
    img.draw_line(0, img.height() / 2, img.width() - 1, img.height() / 2,
                  NORTH_ROAD, 1);
    // img.draw_line(0, img.height() / 2 + 1, img.width() - 1,
    //               img.height() / 2 + 1, MIDDLE_ROAD, 1);
    // img.draw_line(0, img.height() / 2 + 2, img.width() - 1,
    //               img.height() / 2 + 2, SOUTH_ROAD, 1);
    // img.draw_line(img.width() / 2, img.height() / 2, img.width() / 2,
    //               img.height() - 1, WEST_ROAD, 1);
}

int main() {
    srand(time(NULL));

    const int WIN_W = CELLS_X * CELL_SIZE;
    const int WIN_H = CELLS_Y * CELL_SIZE;
    CImg<unsigned char> grid(CELLS_X, CELLS_Y, 1, 3, 0);
    draw_intersection(grid);

    CImgDisplay win(WIN_W, WIN_H, "Simulation grid");

    for (uint64_t t = 0; t < 10000; t += STEP) {
        win.wait(1000);
        if (win.is_closed()) break;
        if (win.resize()) {
            win.resize(WIN_W, WIN_H);
        }

        CImg<unsigned char> zoomed =
            grid.get_resize(WIN_W, WIN_H, -100, -100, 1);
        win.display(zoomed);

        // Spawn cars at 0.5 probability
        // if (rand() % 2 == 1) cars.push_back(Car{(rand() % VMAX) + 1, 0});
        if (t == 0)
            cars.push_back(Car{(rand() % VMAX) + 1, 0});  // TODO: remove

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
            car.speed = std::max(0, car.speed - 1);
        }

        // R4: Car movement
        for (auto& car : cars) {
            car.x += car.speed;
        }

        // Remove cars that are out of bounds
        cars.erase(std::remove_if(cars.begin(), cars.end(),
                                  [grid](const Car& car) {
                                      grid.draw_point(car.x - car.speed,
                                                      CELLS_Y / 2, NORTH_ROAD,
                                                      1);
                                      return car.x >= CELLS_X;
                                  }),
                   cars.end());

        // Draw cars
        for (const auto& car : cars) {
            // Undraw old position
            grid.draw_point(car.x - car.speed, CELLS_Y / 2, NORTH_ROAD, 1);

            // Draw new position
            grid.draw_point(car.x, CELLS_Y / 2, CAR_COLOR, 1);
        }
    }

    // TODO: Get statistics

    return 0;
}
