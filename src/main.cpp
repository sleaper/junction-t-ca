#include <stdlib.h>
#include <time.h>

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "CImg.h"

using namespace cimg_library;

const uint8_t CAR_COLOR[] = {0, 0, 255};
const uint8_t NORTH_ROAD[] = {255, 255, 255};
const uint8_t MIDDLE_ROAD[] = {150, 150, 150};
const uint8_t SOUTH_ROAD[] = {200, 200, 200};
const uint8_t WEST_ROAD[] = {100, 100, 100};
const uint8_t EAST_ROAD[] = {0, 255, 0};
const uint8_t INTERSECTION_COLOR[] = {255, 0, 0};

const int MAIN_LANE_LENGTH = 100;  // in cells
const int MINOR_LANE_LENGTH = 18;
const int SLIP_LANE_LENGTH = 17;

const int SCREEN_CELLS_X = 100;  // 750 / 100 = 7.5m per cell
const int SCREEN_CELLS_Y = 100;

constexpr double CELL_LENGTH_M = 7.5;
const int CELL_SIZE = 10;  // in pixels

const int DELTA = 1;  // 1 second of model time per step
const int VMAX = 9;

const double FLOW_RATE_MAJOR = 0.684;
const double FLOW_RATE_MINOR = 0.2;

const int CAR_SPAWN_DISTANCE = 1;  // Minimum cells gap before spawning

enum class Direction { WEST, EAST, NORTH, SOUTH };
enum class LaneType { THROUGH, TURN, MIXED };

struct Lane;
struct Car;
class IntersectionManager;

struct Car {
    int speed = 0;
    size_t pos = 0;
    Direction direction;
    float aggression = 0.0;
    Lane* lane = nullptr;
    bool wants_to_turn = false;
    size_t id;

    Car(Direction dest, float aggr, Lane* lane, size_t id)
        : direction(dest), aggression(aggr), lane(lane), id(id) {
        speed = static_cast<int>(rand() % VMAX) + 1;
    }

    void draw(CImg<unsigned char>& img) const {
        auto [x, y] = lane->get_position(pos);
        img.draw_point(x, y, CAR_COLOR, 1);
    }
};

struct Lane {
    Direction dir;
    LaneType type;
    std::vector<Car*> cars;
    int len_cels;
    std::string id;
    int start_x, start_y;
    bool is_vertical;

    Lane(Direction dir, LaneType type, int len_cels, std::string id,
         int start_x, int start_y, bool is_vertical)
        : dir(dir),
          type(type),
          len_cels(len_cels),
          id(id),
          start_x(start_x),
          start_y(start_y),
          is_vertical(is_vertical) {}

    std::pair<int, int> get_direction_vector() const {
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

    /**
     * @brief Get the position of the cell on the lane
     * @param pos The position of the cell on the lane, 0 means the start of the
     * line, len_cels means the end of the line
     * @return The position of the cell in pixels
     */
    std::pair<int, int> get_position(size_t pos) const {
        auto [dx, dy] = get_direction_vector();
        return {start_x + dx * static_cast<int>(pos),
                start_y + dy * static_cast<int>(pos)};
    }

    inline const uint8_t* get_road_color() const {
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

    int distance_to_next_car(size_t car_index) const {
        if (car_index >= cars.size() - 1) {
            return -1;
        } else {
            return static_cast<int>(cars.at(car_index + 1)->pos -
                                    cars.at(car_index)->pos - 1);
        }
    }

    Car* find_car_at_pos(size_t pos) const {
        for (auto& car : cars) {
            if (car->pos == pos) return car;
        }
        return nullptr;
    }

    void draw(CImg<unsigned char>& img) const {
        auto [x1, y1] = get_position(0);
        auto [x2, y2] = get_position(len_cels);
        img.draw_line(x1, y1, x2, y2, get_road_color(), 1);
    }
};

class IntersectionManager {
   public:
    std::vector<Lane*> lanes;

    IntersectionManager() {}

    void add_lane(Lane* lane) { lanes.push_back(lane); }

   private:
};

// Global state
std::vector<std::unique_ptr<Lane>> g_lanes;
std::vector<std::unique_ptr<Car>> g_cars;
IntersectionManager intersection_manager;
size_t next_car_id = 0;

void add_lane_to_system(std::unique_ptr<Lane> lane) {
    intersection_manager.add_lane(lane.get());
    g_lanes.push_back(std::move(lane));
}

void init_lanes() {
    int y_mid = SCREEN_CELLS_Y / 2;
    int x_mid = SCREEN_CELLS_X / 2;
    int y_last = SCREEN_CELLS_Y - 1;
    int x_last = SCREEN_CELLS_X - 1;
    int major_lane_offset = 3;
    int slip_lane_offset = 1;

    // Major lanes
    auto west =
        std::make_unique<Lane>(Direction::WEST, LaneType::THROUGH,
                               MAIN_LANE_LENGTH, "west", x_last, y_mid, false);

    auto slip = std::make_unique<Lane>(
        Direction::WEST, LaneType::TURN, SLIP_LANE_LENGTH, "slip",
        x_mid + SLIP_LANE_LENGTH + 1, y_mid + slip_lane_offset, false);

    auto east =
        std::make_unique<Lane>(Direction::EAST, LaneType::THROUGH,
                               MAIN_LANE_LENGTH, "east", 0, y_mid + 2, false);

    auto north = std::make_unique<Lane>(
        Direction::NORTH, LaneType::THROUGH, MINOR_LANE_LENGTH, "north", x_mid,
        y_mid + MINOR_LANE_LENGTH + major_lane_offset, true);

    auto south = std::make_unique<Lane>(Direction::SOUTH, LaneType::THROUGH,
                                        MINOR_LANE_LENGTH, "south", x_mid - 1,
                                        y_mid + major_lane_offset, true);

    add_lane_to_system(std::move(west));
    add_lane_to_system(std::move(slip));
    add_lane_to_system(std::move(east));
    add_lane_to_system(std::move(north));
    add_lane_to_system(std::move(south));
}

void draw(CImg<unsigned char>& img) {
    img.fill(0);

    for (auto& lane : g_lanes) {
        lane->draw(img);
    }

    // Draw intersection TODO: Maybe put into IntersectionManager ??
    for (int dx = 0; dx < 2; dx++) {
        for (int dy = 0; dy < 3; dy++) {
            int x = SCREEN_CELLS_X / 2 + dx - 1;
            int y = SCREEN_CELLS_Y / 2 + dy;
            img.draw_point(x, y, INTERSECTION_COLOR, 1);
        }
    }

    for (auto& car : g_cars) {
        car->draw(img);
    }
}

// void sim_step() {
//     spawn_cars();

//     for (auto& car : g_cars) {
//         attempt_lane_change(car.get());
//     }

//     for (auto& lane : g_lanes) {
//         apply_nash_rules(lane.get());
//     }

//     remove_out_of_bounds_cars();
// }

int main() {
    init_lanes();

    srand(static_cast<unsigned int>(time(nullptr)));

    bool visualize = true;

    const int WIN_W = SCREEN_CELLS_X * CELL_SIZE;
    const int WIN_H = SCREEN_CELLS_Y * CELL_SIZE;
    CImg<unsigned char> grid(SCREEN_CELLS_X, SCREEN_CELLS_Y, 1, 3, 0);

    CImgDisplay win;

    if (visualize) win = CImgDisplay(WIN_W, WIN_H, "Simulation grid");

    for (unsigned long mt = 0; mt < 10000; mt += DELTA) {
        // sim_step();

        if (visualize) {
            draw(grid);
            CImg<unsigned char> zoomed =
                grid.get_resize(WIN_W, WIN_H, -100, -100, 1);
            win.display(zoomed);

            if (win.is_closed()) break;
            if (win.resize()) {
                win.resize(WIN_W, WIN_H);
            }

            win.wait(1000);  // 1 second real-time delay for visualization
        }

        // TODO: Collect statistics
    }

    // TODO: Output statistics

    return 0;
}
