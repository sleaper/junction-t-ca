#ifndef CONFIG_H
#define CONFIG_H
// TODO: for visualisation purposes
constexpr int SCREEN_CELLS_X = 500;
constexpr int SCREEN_CELLS_Y = 500;
constexpr int CELL_PIXELS = 5;       // in pixels
constexpr double CELL_SIZE_M = 0.5;  // in meters

constexpr int DELTA = 1;               // 1 second of model time per step
constexpr int MAIN_LANE_LENGTH = 500;  // in cells
constexpr int VMAX = 60;

constexpr double SLOW_TO_START_PROB = 0.5;  // q
constexpr double RANDOMIZATION_PROB = 0.5;  // p

constexpr int CAR_LEN = 12;
constexpr int CAR_SPAWN_DISTANCE = 12;  // in cells

const unsigned char red[] = {255, 0, 0};

#endif  // CONFIG_H