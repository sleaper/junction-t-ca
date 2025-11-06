#ifndef CAR_H
#define CAR_H

#include <cstddef>
#include <random>

#include "CImg.h"
#include "Lane.h"

using namespace cimg_library;

const int CAR_LENGTH_CELLS = 12;

struct Car {
    int speed = 0;
    size_t pos = 0;
    Direction dir;
    Lane* lane = nullptr;
    size_t id;

    double critical_gap_front;
    double critical_gap_back;

    static std::mt19937 rng;
    static std::lognormal_distribution<double> gap_dist;

    Car(Direction dir, Lane* lane, size_t id);

    void sample_critical_grap();

    int get_accel() const;

    void draw(CImg<unsigned char>& img) const;
};

#endif  // CAR_H
