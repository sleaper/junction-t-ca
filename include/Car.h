#ifndef CAR_H
#define CAR_H

#include "Lane.h"
#include "config.h"

struct Car {
    int id;
    LaneType lane_id = LaneType::Left;
    int poss;
    int v = 0;

    bool aggressive = false;

    int front_gap(const Lane* lane) const {
        int gap = 0;
        int car_pos = poss;

        for (int i = 1; i <= MAIN_LANE_LENGTH; i++) {
            int cell = car_pos + i;
            if (cell >= MAIN_LANE_LENGTH) cell -= MAIN_LANE_LENGTH;
            if (lane->occ.at(cell) != EMPTY_CELL) {
                return gap;
            }
            gap++;
        }

        return gap;
    }

    int back_gap(const Lane* lane) const {
        int gap = 0;
        int car_pos = poss;

        for (int i = 1; i <= MAIN_LANE_LENGTH; i++) {
            int cell = car_pos - i;
            if (cell < 0) cell += MAIN_LANE_LENGTH;
            if (lane->occ.at(cell) != EMPTY_CELL) {
                return gap;
            }
            gap++;
        }

        return gap;
    }
};

#endif  // CAR_H
