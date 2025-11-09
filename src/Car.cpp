#include "Car.h"

#include <stdint.h>
#include <stdlib.h>

#include "Lane.h"

const uint8_t CAR_COLOR[] = {0, 0, 255};

const int ACCEL_HIGH = 4;
const int ACCEL_MED = 3;
const int ACCEL_LOW = 2;

Car::Car(Direction dir, Lane* lane, size_t id) : dir(dir), lane(lane), id(id){};

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
