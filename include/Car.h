#ifndef CAR_H
#define CAR_H

#include <cstddef>

#include "CImg.h"
#include "Lane.h"

using namespace cimg_library;

struct Car {
    int speed = 0;
    size_t pos = 0;
    Direction direction;
    float aggression = 0.0;
    Lane* lane = nullptr;
    bool wants_to_turn = false;
    size_t id;

    Car(Direction dest, float aggr, Lane* lane, bool wants_to_turn, size_t id);

    void draw(CImg<unsigned char>& img) const;
};

#endif  // CAR_H
