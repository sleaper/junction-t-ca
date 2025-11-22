#ifndef CAR_H
#define CAR_H

#include "Lane.h"

struct Car {
    int id;
    LaneType lane_id = LaneType::Left;
    int poss;
    int v = 0;

    bool aggressive = false;
};

#endif  // CAR_H
