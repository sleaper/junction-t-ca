#ifndef CONFIG_H
#define CONFIG_H
// TODO: for visualisation purposes
constexpr int SCREEN_CELLS_X = 500;
constexpr int SCREEN_CELLS_Y = 500;
constexpr int CELL_PIXELS = 5;       // in pixels
constexpr double CELL_SIZE_M = 0.5;  // in meters

constexpr int DELTA = 1;                 // 1 second of model time per step
constexpr int MAIN_LANE_LENGTH = 10000;  // in cells
constexpr int VMAX = 60;

constexpr double SLOW_TO_START_PROB = 0.8;  // q
constexpr double BREAKING_PROB = 0.0;       // p
constexpr double LANE_CHANGE_PROB = 0.8;    // s

constexpr int CAR_LEN = 12;
constexpr int CAR_SPAWN_DISTANCE = 12;  // in cells

constexpr int MAX_TIME_STEP = 3600;
constexpr double SPAWN_DENSITY = 0.1;

// const double STS_PROBS[] = {0.0, 0.2, 0.4, 0.6, 0.8};
// const double BREAKING_PROBS[] = {0.0, 0.2, 0.4, 0.6, 0.8};
const std::vector<double> SPAWN_DENSITIES = {
    0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7};

const unsigned char red[] = {255, 0, 0};

#endif  // CONFIG_H