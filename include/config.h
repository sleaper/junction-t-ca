#ifndef CONFIG_H
#define CONFIG_H

constexpr double CELL_SIZE_M = 7.5;     // in meters
constexpr int MAIN_LANE_LENGTH = 1200;  // in cells
constexpr int DELTA = 1;                // 1 second of model time per step
constexpr int VMAX = 5;

constexpr int LOOK_AHEAD = 5;
constexpr int LOOK_OTHER_BACK = 5;

constexpr double BREAKING_PROB = 0.5;
constexpr double LANE_CHANGE_PROB = 1;

constexpr int MAX_TIME_STEP = 5000;
constexpr int WARMUP_STEPS = 1000;

#endif  // CONFIG_H
