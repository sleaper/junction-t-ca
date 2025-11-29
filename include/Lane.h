#ifndef LANE_H
#define LANE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "CImg.h"

using namespace cimg_library;

struct Car;
const int EMPTY_CELL = -1;
const uint8_t ROAD_COLOR[] = {255, 255, 255};
enum class LaneType { Left = 0, Right = 1 };

struct Lane {
    LaneType id;
    int start_x, start_y;

    std::vector<int> occ;       // t
    std::vector<int> next_occ;  // t+1

    Lane(LaneType id, int start_x, int start_y);

    void clear_next();
    void swap_buffers();

    std::pair<int, int> screen_coords(size_t pos) const;
    void draw(CImg<unsigned char>& img) const;
};

#endif  // LANE_H
