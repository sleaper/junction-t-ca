#include "Lane.h"

#include <stdint.h>

#include <cmath>
#include <stdexcept>

#include "config.h"

using namespace cimg_library;

Lane::Lane(int id_, int start_x_, int start_y_)
    : id(id_),
      start_x(start_x_),
      start_y(start_y_),
      occ(MAIN_LANE_LENGTH, EMPTY_CELL),
      next_occ(MAIN_LANE_LENGTH, EMPTY_CELL),
      gap_ahead(MAIN_LANE_LENGTH, 0),
      gap_behind(MAIN_LANE_LENGTH, 0) {}

std::pair<int, int> Lane::screen_coords(size_t pos) const {
    return {start_x + static_cast<int>(pos), start_y};
}

void Lane::clear_next() {
    std::fill(next_occ.begin(), next_occ.end(), EMPTY_CELL);
}

void Lane::swap_buffers() {
    occ.swap(next_occ);
    clear_next();
}

void Lane::draw(CImg<unsigned char>& img) const {
    auto [x1, y1] = screen_coords(0);
    auto [x2, y2] = screen_coords(MAIN_LANE_LENGTH - 1);
    img.draw_line(x1, y1, x2, y2, ROAD_COLOR, 1);

    for (const auto& car_id : occ) {
        if (car_id != EMPTY_CELL) {
            auto [cx, cy] = screen_coords(&car_id - &occ[0]);
            // img.draw_rectangle(cx, cy, cx + CAR_LEN, cy, red, 3);
            img.draw_point(cx, cy, red, 3);
        }
    }
}
