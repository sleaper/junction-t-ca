#ifndef LANE_H
#define LANE_H

#include <string>
#include <utility>
#include <vector>

#include "CImg.h"

using namespace cimg_library;

struct Car;

struct Lane {
    int id;
    int start_x, start_y;

    std::vector<int> occ;       // t
    std::vector<int> next_occ;  // t+1

    std::vector<int> gap_ahead, gap_behind;

    Lane(int id, int start_x, int start_y);

    void clear_next();
    void swap_buffers();

    std::pair<int, int> screen_coords(size_t pos) const;
    void draw(CImg<unsigned char>& img) const;
};

#endif  // LANE_H
