#include "Car.h"

#include <stdlib.h>

const uint8_t CAR_COLOR[] = {0, 0, 255};
const uint8_t LANE_CHANGER_COLOR[] = {255, 0, 0};

Car::Car(Direction dest, float aggr, Lane* lane, bool wants_to_turn, size_t id)
    : direction(dest),
      aggression(aggr),
      lane(lane),
      wants_to_turn(wants_to_turn),
      id(id) {
    // Somehow decide how fast they should be going
    speed = static_cast<int>(rand() % VMAX) + 1;
}

void Car::draw(CImg<unsigned char>& img) const {
    auto [x, y] = lane->get_position(pos);

    if (wants_to_turn) {
        img.draw_point(x, y, LANE_CHANGER_COLOR, 1);
    } else {
        img.draw_point(x, y, CAR_COLOR, 1);
    }
}
