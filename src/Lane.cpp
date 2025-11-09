#include "Lane.h"

#include <stdint.h>

#include <cmath>
#include <stdexcept>

#include "config.h"

using namespace cimg_library;

const uint8_t ROAD_COLOR[] = {255, 255, 255};
const int EMPTY = -1;

Lane::Lane(int len_cels_, int id_, int start_x_, int start_y_)
    : len_cels(len_cels_),
      id(id_),
      start_x(start_x_),
      start_y(start_y_),
      occ(len_cels_, EMPTY),
      next_occ(len_cels_, EMPTY),
      gap_ahead(len_cels_, 0),
      gap_behind(len_cels_, 0) {}

std::pair<int, int> Lane::screen_coords(size_t pos) const {
    return {start_x + static_cast<int>(pos), start_y};
}

void Lane::clear_next() { std::fill(next_occ.begin(), next_occ.end(), EMPTY); }

void Lane::swap_buffers() {
    occ.swap(next_occ);
    clear_next();
}

void Lane::draw(CImg<unsigned char>& img) const {
    auto [x1, y1] = screen_coords(0);
    auto [x2, y2] = screen_coords(len_cels);
    img.draw_line(x1, y1, x2, y2, ROAD_COLOR, 1);

    for (const auto& car_id : occ) {
        if (car_id != EMPTY) {
            auto [cx, cy] = screen_coords(&car_id - &occ[0]);
            // img.draw_rectangle(cx, cy, cx + CAR_LEN, cy, red, 3);
            img.draw_point(cx, cy, red, 3);
        }
    }
}
