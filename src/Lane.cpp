#include "Lane.h"

#include <stdexcept>

#include "Car.h"

// Road color constants
const uint8_t NORTH_ROAD[] = {255, 255, 255};
const uint8_t MIDDLE_ROAD[] = {150, 150, 150};
const uint8_t SOUTH_ROAD[] = {200, 200, 200};
const uint8_t WEST_ROAD[] = {100, 100, 100};
const uint8_t EAST_ROAD[] = {0, 255, 0};

Lane::Lane(Direction dir, LaneType type, int len_cels, std::string id,
           int start_x, int start_y, bool is_vertical)
    : dir(dir),
      type(type),
      len_cels(len_cels),
      id(id),
      start_x(start_x),
      start_y(start_y),
      is_vertical(is_vertical) {}

std::pair<int, int> Lane::get_direction_vector() const {
    switch (dir) {
        case Direction::WEST:
            return {-1, 0};
        case Direction::EAST:
            return {1, 0};
        case Direction::NORTH:
            return {0, -1};
        case Direction::SOUTH:
            return {0, 1};
        default:
            throw std::invalid_argument("Invalid direction");
    }
}

/*
 * Translates a position in the lane vector into (x) coordinates on the screen
 * (x, y).
 */
std::pair<int, int> Lane::get_position(size_t pos) const {
    auto [dx, dy] = get_direction_vector();
    return {start_x + dx * static_cast<int>(pos),
            start_y + dy * static_cast<int>(pos)};
}

const uint8_t* Lane::get_road_color() const {
    switch (dir) {
        case Direction::WEST:
            return WEST_ROAD;
        case Direction::NORTH:
            return NORTH_ROAD;
        case Direction::EAST:
            return EAST_ROAD;
        case Direction::SOUTH:
            return SOUTH_ROAD;
        default:
            throw std::invalid_argument("Invalid direction");
    }
}

int Lane::distance_to_next_car(size_t car_index) const {
    if (car_index >= cars.size() - 1) {
        return -1;
    } else {
        return static_cast<int>(cars.at(car_index + 1)->pos -
                                cars.at(car_index)->pos - 1);
    }
}

Car* Lane::find_car_at_pos(size_t pos) const {
    for (auto& car : cars) {
        if (car->pos == pos) return car;
    }
    return nullptr;
}

void Lane::draw(CImg<unsigned char>& img) const {
    auto [x1, y1] = get_position(0);
    auto [x2, y2] = get_position(len_cels);
    img.draw_line(x1, y1, x2, y2, get_road_color(), 1);
}
