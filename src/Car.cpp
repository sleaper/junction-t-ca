#include "Car.h"

#include <stdlib.h>

const uint8_t CAR_COLOR[] = {0, 0, 255};

Car::Car(Direction dest, float aggr, Lane* lane, size_t id)
    : direction(dest), aggression(aggr), lane(lane), id(id) {
    speed = static_cast<int>(rand() % VMAX) + 1;
}

void Car::draw(CImg<unsigned char>& img) const {
    auto [x, y] = lane->get_position(pos);
    img.draw_point(x, y, CAR_COLOR, 1);
}
