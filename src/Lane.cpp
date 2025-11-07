#include "Lane.h"

#include <stdexcept>

#include "Car.h"

const uint8_t ROAD_COLOR[] = {255, 255, 255};

Lane::Lane(Direction dir, int len_cels, std::string id, int start_x,
           int start_y)
    : dir(dir),
      len_cels(len_cels),
      id(id),
      start_x(start_x),
      start_y(start_y) {}

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
        case Direction::ANY:
            return {1, 1};
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

int Lane::target_front_gap(size_t car_index, Lane* target_lane) const {
    Car* car = cars[car_index];
    for (size_t i = 0; i < target_lane->cars.size(); i++) {
        Car* target_car = target_lane->cars[i];
        if (target_car->pos > car->pos) {
            return target_car->pos - car->pos - CAR_LENGTH_CELLS;
        }
    }
    return -1;
}

int Lane::target_back_gap(size_t car_index, Lane* target_lane) const {
    Car* car = cars[car_index];
    for (int i = static_cast<int>(target_lane->cars.size()) - 1; i >= 0; i--) {
        Car* target_car = target_lane->cars[i];
        if (target_car->pos < car->pos) {
            return car->pos - target_car->pos - CAR_LENGTH_CELLS;
        }
    }
    return -1;
}

int Lane::front_gap(size_t car_index) const {
    if (car_index + 1 >= cars.size()) {
        return -1;
    } else {
        return cars[car_index + 1]->pos - cars[car_index]->pos -
               CAR_LENGTH_CELLS;
    }
}

int Lane::back_gap(size_t car_index) const {
    if (car_index == 0) {
        return -1;
    } else {
        return cars[car_index]->pos - cars[car_index - 1]->pos -
               CAR_LENGTH_CELLS;
    }
}

void Lane::insert_car(Car* car) {
    auto it = std::lower_bound(
        cars.begin(), cars.end(), car,
        [](const Car* a, const Car* b) { return a->pos < b->pos; });
    cars.insert(it, car);
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
    img.draw_line(x1, y1, x2, y2, ROAD_COLOR, 1);

    for (const& auto car : cars) {
        car->draw(img);
    }
}
