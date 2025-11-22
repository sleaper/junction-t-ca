#ifndef CONFIG_H
#define CONFIG_H
// TODO: for visualisation purposes
constexpr int SCREEN_CELLS_X = 800;
constexpr int SCREEN_CELLS_Y = 200;
constexpr int CELL_PIXELS = 10;      // in pixels
constexpr double CELL_SIZE_M = 7.5;  // in meters

constexpr int DELTA = 1;                 // 1 second of model time per step
constexpr int MAIN_LANE_LENGTH = 12000;  // in cells
constexpr int VMAX = 5;
constexpr int LOOK_AHEAD = 5;
constexpr int LOOK_OTHER_BACK = 5;

constexpr double BREAKING_PROB = 0.5;
constexpr double LANE_CHANGE_PROB = 1;

constexpr int MAX_TIME_STEP = 5000;
constexpr int WARMUP_STEPS = 1000;

enum class MODE_TYPE { SYMMETRIC, ASYMMETRIC };
constexpr MODE_TYPE MODE = MODE_TYPE::SYMMETRIC;

const unsigned char red[] = {255, 0, 0};

#endif  // CONFIG_H
