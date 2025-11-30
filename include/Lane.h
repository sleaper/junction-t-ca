#ifndef LANE_H
#define LANE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct Car;
const int EMPTY_CELL = -1;
enum class LaneType { Left = 0, Right = 1 };

struct Lane {
    LaneType id;

    std::vector<int> occ;       // t
    std::vector<int> next_occ;  // t+1

    Lane(LaneType id);

    void clear_next();
    void swap_buffers();
};

#endif  // LANE_H
