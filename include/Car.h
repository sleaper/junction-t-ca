#ifndef CAR_H
#define CAR_H

#include <cstddef>

#include "CImg.h"
#include "Lane.h"

using namespace cimg_library;

const int CAR_LENGTH_CELLS = 12;
constexpr double CELL_LENGTH_M = 0.5;

struct Car {
    int speed = 0;
    int pos;
    Direction dir;
    Lane* lane = nullptr;
    size_t id;

    int prev_front_dist;
    int prev_back_dist;

    Car(Direction dir, Lane* lane, size_t id);

    int get_accel() const;

    void draw(CImg<unsigned char>& img) const;
};

#endif  // CAR_H
