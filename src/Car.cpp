#include "Car.h"

#include <stdlib.h>

#include "Lane.h"

// Define static member variables
std::mt19937 Car::rng;
std::lognormal_distribution<double> Car::gap_dist;

const uint8_t CAR_COLOR[] = {0, 0, 255};
constexpr double CELL_LENGTH_M = 0.5;
const int ACCEL_HIGH = 4;
const int ACCEL_MED = 3;
const int ACCEL_LOW = 2;

Car::Car(Direction dir, Lane* lane, size_t id) : dir(dir), lane(lane), id(id) {
    sample_critical_grap();
}

void Car::sample_critical_grap() {
    double front_gap_m = gap_dist(rng);
    double back_gap_m = gap_dist(rng) * 0.7;  // TODO:

    critical_gap_front = std::ceil(front_gap_m / CELL_LENGTH_M);
    critical_gap_back = std::ceil(back_gap_m / CELL_LENGTH_M);
}

int Car::get_accel() const {
    if (speed < 20) {
        return ACCEL_HIGH;
    } else if (speed < 40) {
        return ACCEL_MED;
    } else {
        return ACCEL_LOW;
    }
}

void Car::draw(CImg<unsigned char>& img) const {
    auto [x, y] = lane->get_position(pos);
    img.draw_point(x, y, CAR_COLOR, 1);
}
