#ifndef CONFIG_H
#define CONFIG_H
// TODO: for visualisation purposes
constexpr int SCREEN_CELLS_X = 500;
constexpr int SCREEN_CELLS_Y = 500;
constexpr int CELL_PIXELS = 5;       // in pixels
constexpr double CELL_SIZE_M = 7.5;  // in meters

constexpr int DELTA = 1;               // 1 second of model time per step
constexpr int MAIN_LANE_LENGTH = 133;  // in cells
constexpr int VMAX = 5;
constexpr int CAR_LEN = 1;

constexpr double LANE_CHANGE_PROB = 1;
constexpr int LOOK_OTHER_AHEAD = 5;
constexpr double BREAKING_PROB = 0.5;

constexpr int MAX_TIME_STEP = 5000;
constexpr double SPAWN_DENSITY = 0.1;

const enum MODE_TYPE { SYMMETRIC, ASYMMETRIC };
const MODE_TYPE MODE = SYMMETRIC;

const unsigned char red[] = {255, 0, 0};

#endif  // CONFIG_H