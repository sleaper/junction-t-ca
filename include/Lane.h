#ifndef LANE_H
#define LANE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "CImg.h"

using namespace cimg_library;

// External constants
extern const int VMAX;

enum class Direction { WEST, EAST, NORTH, SOUTH, ANY };

struct Car;  // Forward declaration

struct Lane {
    Direction dir;
    std::vector<Car*> cars;
    size_t len_cels;
    std::string id;
    int start_x, start_y;

    Lane(Direction dir, int len_cels, std::string id, int start_x, int start_y);

    int measure_front_gap(size_t car_index) const;
    int measure_back_gap(size_t car_index) const;
    int measure_target_front_gap(size_t car_index) const;
    int measure_target_back_gap(size_t car_index) const;

    std::pair<int, int> get_direction_vector() const;
    std::pair<int, int> get_position(size_t pos) const;
    const uint8_t* get_road_color() const;
    int distance_to_next_car(size_t car_index) const;
    Car* find_car_at_pos(size_t pos) const;
    void draw(CImg<unsigned char>& img) const;
};

#endif  // LANE_H
