#include "Lane.h"

#include <stdint.h>

#include <cmath>
#include <stdexcept>

#include "config.h"

Lane::Lane(LaneType id_)
    : id(id_),
      occ(MAIN_LANE_LENGTH, EMPTY_CELL),
      next_occ(MAIN_LANE_LENGTH, EMPTY_CELL) {}

void Lane::clear_next() {
    std::fill(next_occ.begin(), next_occ.end(), EMPTY_CELL);
}

void Lane::swap_buffers() {
    occ.swap(next_occ);
    clear_next();
}
