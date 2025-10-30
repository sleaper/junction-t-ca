#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "CImg.h"
#include "Car.h"
#include "Lane.h"

using namespace cimg_library;

// Paper parameters: L=2000, l=2, v_max=6, p=0.3
const int MAIN_LANE_LENGTH = 2000;
const int MINOR_LANE_LENGTH = 1000;
const int CAR_LENGTH = 2;

const int VMAX = 6;                     // v_max = 6
const double RANDOMIZATION_PROB = 0.3;  // p = 0.3

// Flow rates (alpha values)
const double ALPHA_A = 0.2;  // Major road A inflow
const double ALPHA_B = 0.2;  // Major road B inflow
const double ALPHA_C = 0.2;  // Minor road C inflow
const double P_T = 0.2;      // Probability of left turn from A

// Detector position for measuring flux
const int DETECTOR_POS = 500;

// Warm-up and measurement periods
const int WARMUP_STEPS = 50000;
const int MEASUREMENT_STEPS = 10000;

// Screen settings for visualization
const int SCREEN_CELLS_X = 200;
const int SCREEN_CELLS_Y = 200;
const int CELL_SIZE = 10;

// T-intersection structure
struct TIntersection {
    size_t T1_pos;  // First T-cell
    size_t T2_pos;  // Second T-cell
    size_t T3_pos;  // Third T-cell
    size_t T4_pos;  // Fourth T-cell

    size_t intersection_start_A;  // Where intersection begins on lane A
    size_t intersection_start_B;  // Where intersection begins on lane B
    size_t intersection_start_C;  // Where intersection begins on lane C
};

struct UpstreamVehicle {
    Car* car = nullptr;
    size_t head_pos = 0;
    int speed = 0;
    int vehicle_type = 0;
    Lane* lane = nullptr;
};

struct FluxCounters {
    int q_A = 0;
    int q_B = 0;
    int q_C = 0;
    int q_D = 0;
};

std::vector<std::unique_ptr<Lane>> g_lanes;
std::vector<std::unique_ptr<Car>> g_cars;
size_t next_car_id = 0;
TIntersection t_intersection;
FluxCounters flux_counters;
bool is_warmup_complete = false;

int destination_to_vehicle_type(Destination dest, Direction lane_dir) {
    if (lane_dir == Direction::WEST) {
        if (dest == Destination::SOUTH) return 1;
        return 0;
    } else if (lane_dir == Direction::NORTH) {
        if (dest == Destination::WEST) return 2;
        return 0;
    }
    return 0;  // Default: straight
}

Lane* find_lane_by_id(const std::string& id) {
    for (auto& lane : g_lanes) {
        if (lane->id == id) return lane.get();
    }
    return nullptr;
}

void init_intersection_geometry() {
    t_intersection.intersection_start_A = MAIN_LANE_LENGTH / 2;
    t_intersection.intersection_start_B = MAIN_LANE_LENGTH / 2;
    t_intersection.intersection_start_C = MINOR_LANE_LENGTH / 2;

    t_intersection.T1_pos = t_intersection.intersection_start_A;
    t_intersection.T2_pos = t_intersection.T1_pos + 1;
    t_intersection.T3_pos = t_intersection.T2_pos + 1;
    t_intersection.T4_pos = t_intersection.T3_pos + 1;
}

void init_lanes() {
    auto lane_A =
        std::make_unique<Lane>(Direction::WEST, MAIN_LANE_LENGTH, "A",
                               SCREEN_CELLS_X - 1, SCREEN_CELLS_Y / 2);

    auto lane_B = std::make_unique<Lane>(Direction::EAST, MAIN_LANE_LENGTH, "B",
                                         0, SCREEN_CELLS_Y / 2 + 2);

    auto lane_C =
        std::make_unique<Lane>(Direction::NORTH, MINOR_LANE_LENGTH, "C",
                               SCREEN_CELLS_X / 2, SCREEN_CELLS_Y - 1);

    auto lane_D = std::make_unique<Lane>(Direction::SOUTH, MINOR_LANE_LENGTH,
                                         "D", SCREEN_CELLS_X / 2 - 1, 0);

    g_lanes.push_back(std::move(lane_A));
    g_lanes.push_back(std::move(lane_B));
    g_lanes.push_back(std::move(lane_C));
    g_lanes.push_back(std::move(lane_D));

    init_intersection_geometry();
}

// Find the first upstream vehicle approaching the intersection on a lane
UpstreamVehicle find_first_upstream_vehicle(Lane* lane,
                                            size_t intersection_start) {
    UpstreamVehicle result;

    if (!lane) return result;

    // Find the car closest to intersection but before it
    for (Car* car : lane->cars) {
        if (car->pos < intersection_start) {
            // This car is before the intersection
            if (result.car == nullptr || car->pos > result.head_pos) {
                result.car = car;
                result.head_pos = car->pos;
                result.speed = car->speed;
                result.vehicle_type =
                    destination_to_vehicle_type(car->destination, lane->dir);
                result.lane = lane;
            }
        }
    }

    return result;
}

// Calculate time to reach a conflict point
double time_to_conflict(const UpstreamVehicle& vehicle,
                        size_t conflict_cell_pos) {
    if (vehicle.car == nullptr) return 1e9;  // Very large number if no vehicle

    int distance = static_cast<int>(conflict_cell_pos) -
                   static_cast<int>(vehicle.head_pos);
    if (distance <= 0) return 0;  // Already at or past conflict point

    int effective_speed = std::max(vehicle.speed, 1);  // Floor speed at 1
    return static_cast<double>(distance) / effective_speed;
}

// Check if cell is occupied by any vehicle on a lane
bool is_cell_occupied(size_t cell_pos, Lane* lane) {
    if (!lane) return false;

    for (Car* car : lane->cars) {
        // Check if car occupies this cell (considering car length)
        for (int i = 0; i < CAR_LENGTH; i++) {
            if (car->pos + i == cell_pos) {
                return true;
            }
        }
    }
    return false;
}

// Apply Rule 2: Time-to-conflict priority
void apply_collision_avoidance_rule2() {
    Lane* lane_A = find_lane_by_id("A");  // Major road west
    Lane* lane_B = find_lane_by_id("B");  // Major road east
    Lane* lane_C = find_lane_by_id("C");  // Minor road north

    if (!lane_A || !lane_B || !lane_C) return;

    // Find first upstream vehicles
    UpstreamVehicle veh_A = find_first_upstream_vehicle(
        lane_A, t_intersection.intersection_start_A);
    UpstreamVehicle veh_B = find_first_upstream_vehicle(
        lane_B, t_intersection.intersection_start_B);
    UpstreamVehicle veh_C = find_first_upstream_vehicle(
        lane_C, t_intersection.intersection_start_C);

    // Conflict 1: Left turn from A (s=1) vs Straight on B (s=0)
    if (veh_A.car && veh_A.vehicle_type == 1 && veh_B.car &&
        veh_B.vehicle_type == 0) {
        // Both compete for T3
        double t_A = time_to_conflict(veh_A, t_intersection.T3_pos);
        double t_B = time_to_conflict(veh_B, t_intersection.T3_pos);

        // Only check if both will arrive soon (within 2 time steps)
        if (t_A < 2.0 && t_B < 2.0) {
            if (t_B < t_A) {
                // B arrives first, A must brake
                int safe_speed =
                    static_cast<int>(t_intersection.T3_pos - veh_A.head_pos) -
                    1;
                veh_A.car->speed =
                    std::max(0, std::min(veh_A.car->speed, safe_speed));
            } else if (t_A < t_B) {
                // A arrives first, B must brake
                int safe_speed =
                    static_cast<int>(t_intersection.T3_pos - veh_B.head_pos) -
                    1;
                veh_B.car->speed =
                    std::max(0, std::min(veh_B.car->speed, safe_speed));
            } else {
                // Tie: 50/50 coin flip
                if (rand() % 2 == 0) {
                    int safe_speed = static_cast<int>(t_intersection.T3_pos -
                                                      veh_A.head_pos) -
                                     1;
                    veh_A.car->speed =
                        std::max(0, std::min(veh_A.car->speed, safe_speed));
                } else {
                    int safe_speed = static_cast<int>(t_intersection.T3_pos -
                                                      veh_B.head_pos) -
                                     1;
                    veh_B.car->speed =
                        std::max(0, std::min(veh_B.car->speed, safe_speed));
                }
            }
        }
    }

    // Conflict 2: Straight on A (s=0) vs Right turn from C (s=2)
    if (veh_A.car && veh_A.vehicle_type == 0 && veh_C.car &&
        veh_C.vehicle_type == 2) {
        // Both compete for T2
        double t_A = time_to_conflict(veh_A, t_intersection.T2_pos);
        double t_C = time_to_conflict(veh_C, t_intersection.T2_pos);

        if (t_A < 2.0 && t_C < 2.0) {
            if (t_C < t_A) {
                int safe_speed =
                    static_cast<int>(t_intersection.T2_pos - veh_A.head_pos) -
                    1;
                veh_A.car->speed =
                    std::max(0, std::min(veh_A.car->speed, safe_speed));
            } else if (t_A < t_C) {
                int safe_speed =
                    static_cast<int>(t_intersection.T2_pos - veh_C.head_pos) -
                    1;
                veh_C.car->speed =
                    std::max(0, std::min(veh_C.car->speed, safe_speed));
            } else {
                if (rand() % 2 == 0) {
                    int safe_speed = static_cast<int>(t_intersection.T2_pos -
                                                      veh_A.head_pos) -
                                     1;
                    veh_A.car->speed =
                        std::max(0, std::min(veh_A.car->speed, safe_speed));
                } else {
                    int safe_speed = static_cast<int>(t_intersection.T2_pos -
                                                      veh_C.head_pos) -
                                     1;
                    veh_C.car->speed =
                        std::max(0, std::min(veh_C.car->speed, safe_speed));
                }
            }
        }
    }

    // Conflict 3: Right turn from C (s=2) vs Straight on B (s=0)
    if (veh_C.car && veh_C.vehicle_type == 2 && veh_B.car &&
        veh_B.vehicle_type == 0) {
        // Both compete for T4
        double t_C = time_to_conflict(veh_C, t_intersection.T4_pos);
        double t_B = time_to_conflict(veh_B, t_intersection.T4_pos);

        if (t_C < 2.0 && t_B < 2.0) {
            if (t_B < t_C) {
                int safe_speed =
                    static_cast<int>(t_intersection.T4_pos - veh_C.head_pos) -
                    1;
                veh_C.car->speed =
                    std::max(0, std::min(veh_C.car->speed, safe_speed));
            } else if (t_C < t_B) {
                int safe_speed =
                    static_cast<int>(t_intersection.T4_pos - veh_B.head_pos) -
                    1;
                veh_B.car->speed =
                    std::max(0, std::min(veh_B.car->speed, safe_speed));
            } else {
                if (rand() % 2 == 0) {
                    int safe_speed = static_cast<int>(t_intersection.T4_pos -
                                                      veh_C.head_pos) -
                                     1;
                    veh_C.car->speed =
                        std::max(0, std::min(veh_C.car->speed, safe_speed));
                } else {
                    int safe_speed = static_cast<int>(t_intersection.T4_pos -
                                                      veh_B.head_pos) -
                                     1;
                    veh_B.car->speed =
                        std::max(0, std::min(veh_B.car->speed, safe_speed));
                }
            }
        }
    }

    // Gridlock avoidance
    bool T3_occupied = is_cell_occupied(t_intersection.T3_pos, lane_B);
    bool T4_occupied = is_cell_occupied(t_intersection.T4_pos, lane_C);

    if (T3_occupied && veh_C.car && veh_C.vehicle_type == 2) {
        // If T3 will be taken by B-straight, C-right may not enter
        int safe_speed =
            static_cast<int>(t_intersection.T4_pos - veh_C.head_pos) - 1;
        veh_C.car->speed = std::max(0, std::min(veh_C.car->speed, safe_speed));
    }

    if (T4_occupied && veh_B.car && veh_B.vehicle_type == 0) {
        // If T4 is taken by C-right, B-straight may not enter
        int safe_speed = static_cast<int>(t_intersection.intersection_start_B -
                                          veh_B.head_pos) -
                         1;
        veh_B.car->speed = std::max(0, std::min(veh_B.car->speed, safe_speed));
    }
}

void spawn_cars() {
    Lane* lane_A = find_lane_by_id("A");
    Lane* lane_B = find_lane_by_id("B");
    Lane* lane_C = find_lane_by_id("C");

    // Spawn on lane A with probability ALPHA_A
    if (lane_A && static_cast<double>(rand()) / RAND_MAX < ALPHA_A) {
        // Check if spawn position is free (need CAR_LENGTH cells free)
        bool can_spawn = true;
        for (int i = 0; i < CAR_LENGTH; i++) {
            if (lane_A->find_car_at_pos(i) != nullptr) {
                can_spawn = false;
                break;
            }
        }

        if (can_spawn) {
            Destination dest = Destination::STRAIGHT;

            // With probability P_T, make it a left-turner (A→D)
            if (static_cast<double>(rand()) / RAND_MAX < P_T) {
                dest = Destination::SOUTH;  // Left turn to south lane
            }

            float aggression = 0.5;  // Not used in this model
            auto car = std::make_unique<Car>(Direction::WEST, aggression,
                                             lane_A, dest, next_car_id++);
            car->speed = 0;  // Start from rest
            car->pos = 0;

            lane_A->cars.insert(lane_A->cars.begin(), car.get());
            g_cars.push_back(std::move(car));
        }
    }

    // Spawn on lane B with probability ALPHA_B
    if (lane_B && static_cast<double>(rand()) / RAND_MAX < ALPHA_B) {
        bool can_spawn = true;
        for (int i = 0; i < CAR_LENGTH; i++) {
            if (lane_B->find_car_at_pos(i) != nullptr) {
                can_spawn = false;
                break;
            }
        }

        if (can_spawn) {
            Destination dest = Destination::STRAIGHT;  // B only goes straight

            float aggression = 0.5;
            auto car = std::make_unique<Car>(Direction::EAST, aggression,
                                             lane_B, dest, next_car_id++);
            car->speed = 0;
            car->pos = 0;

            lane_B->cars.insert(lane_B->cars.begin(), car.get());
            g_cars.push_back(std::move(car));
        }
    }

    // Spawn on lane C with probability ALPHA_C
    if (lane_C && static_cast<double>(rand()) / RAND_MAX < ALPHA_C) {
        bool can_spawn = true;
        for (int i = 0; i < CAR_LENGTH; i++) {
            if (lane_C->find_car_at_pos(i) != nullptr) {
                can_spawn = false;
                break;
            }
        }

        if (can_spawn) {
            // For now, assume C vehicles are right-turners (C→A)
            Destination dest = Destination::WEST;  // Right turn to lane A

            float aggression = 0.5;
            auto car = std::make_unique<Car>(Direction::NORTH, aggression,
                                             lane_C, dest, next_car_id++);
            car->speed = 0;
            car->pos = 0;

            lane_C->cars.insert(lane_C->cars.begin(), car.get());
            g_cars.push_back(std::move(car));
        }
    }
}

void measure_flux() {
    if (!is_warmup_complete) return;

    // Count cars passing detector at position DETECTOR_POS
    for (auto& car : g_cars) {
        // Check if car just crossed detector position this step
        if (car->pos >= DETECTOR_POS &&
            (car->pos - car->speed) < DETECTOR_POS) {
            if (car->lane->id == "A")
                flux_counters.q_A++;
            else if (car->lane->id == "B")
                flux_counters.q_B++;
            else if (car->lane->id == "C")
                flux_counters.q_C++;
            else if (car->lane->id == "D")
                flux_counters.q_D++;
        }
    }
}

void remove_out_of_bounds_cars() {
    g_cars.erase(
        std::remove_if(g_cars.begin(), g_cars.end(),
                       [](const std::unique_ptr<Car>& car) {
                           if (car->pos >= car->lane->len_cels) {
                               // Also remove from lane
                               auto& lane_cars = car->lane->cars;
                               lane_cars.erase(
                                   std::remove(lane_cars.begin(),
                                               lane_cars.end(), car.get()),
                                   lane_cars.end());
                               return true;
                           }
                           return false;
                       }),
        g_cars.end());
}

void apply_nasch_rules() {
    for (auto& car : g_cars) {
        // R1: Acceleration
        if (car->speed < VMAX) {
            car->speed += 1;
        }

        // R2: Deceleration due to other cars
        auto& lane_cars = car->lane->cars;
        auto it = std::find(lane_cars.begin(), lane_cars.end(), car.get());
        if (it != lane_cars.end()) {
            size_t car_index = std::distance(lane_cars.begin(), it);

            // Calculate distance to next car
            if (car_index < lane_cars.size() - 1) {
                Car* next_car = lane_cars[car_index + 1];
                int distance =
                    static_cast<int>(next_car->pos - car->pos) - CAR_LENGTH;
                if (distance >= 0) {
                    car->speed = std::min(car->speed, distance);
                }
            } else {
                // No car ahead, check distance to end of lane
                int distance =
                    static_cast<int>(car->lane->len_cels - car->pos) -
                    CAR_LENGTH;
                if (distance >= 0) {
                    car->speed = std::min(car->speed, distance);
                }
            }
        }

        // R3: Randomization with probability p=0.3
        if (static_cast<double>(rand()) / RAND_MAX < RANDOMIZATION_PROB) {
            car->speed = std::max(0, car->speed - 1);
        }
    }
}

void apply_movement() {
    // R4: Move all cars
    for (auto& car : g_cars) {
        car->pos += car->speed;
    }
}

void sim_step(unsigned long step) {
    if (step == WARMUP_STEPS) {
        is_warmup_complete = true;
        std::cout << "Warmup complete at step " << step << std::endl;
    }

    spawn_cars();

    apply_nasch_rules();

    // Apply collision avoidance BEFORE movement
    apply_collision_avoidance_rule2();

    // Apply movement (step 4)
    apply_movement();

    // Measure flux if past warmup
    measure_flux();

    // Remove cars that have left the simulation
    remove_out_of_bounds_cars();
}

void draw(CImg<unsigned char>& img) {
    img.fill(0);

    for (auto& lane : g_lanes) {
        lane->draw(img);
    }

    for (auto& car : g_cars) {
        car->draw(img);
    }
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));

    init_lanes();

    bool visualize = false;  // Set to false for headless mode
    if (argc > 1 && std::string(argv[1]) == "--visualize") {
        visualize = true;
    }

    const int WIN_W = SCREEN_CELLS_X * CELL_SIZE;
    const int WIN_H = SCREEN_CELLS_Y * CELL_SIZE;
    CImg<unsigned char> grid(SCREEN_CELLS_X, SCREEN_CELLS_Y, 1, 3, 0);
    CImgDisplay* win = nullptr;

    if (visualize) {
        win = new CImgDisplay(WIN_W, WIN_H, "T-Intersection Simulation");
    }

    int total_steps = WARMUP_STEPS + MEASUREMENT_STEPS;

    std::cout << "Starting simulation..." << std::endl;
    std::cout << "Parameters: L=" << MAIN_LANE_LENGTH << ", v_max=" << VMAX
              << ", p=" << RANDOMIZATION_PROB << std::endl;
    std::cout << "Alpha_A=" << ALPHA_A << ", Alpha_B=" << ALPHA_B
              << ", Alpha_C=" << ALPHA_C << ", P_T=" << P_T << std::endl;
    std::cout << "Warmup steps: " << WARMUP_STEPS
              << ", Measurement steps: " << MEASUREMENT_STEPS << std::endl;

    for (unsigned long step = 0; step < total_steps; step++) {
        sim_step(step);

        if (visualize && win) {
            if (win->is_closed()) break;

            draw(grid);
            CImg<unsigned char> zoomed =
                grid.get_resize(WIN_W, WIN_H, -100, -100, 1);

            std::string iter_text = "Step: " + std::to_string(step);
            const unsigned char white[] = {255, 255, 255};
            zoomed.draw_text(10, 10, iter_text.c_str(), white, 0, 1, 24);

            win->display(zoomed);
            win->wait(1000);
        }

        // Progress report every 10000 steps
        if (step % 10000 == 0 && step > 0) {
            std::cout << "Step " << step << " - Cars: " << g_cars.size()
                      << std::endl;
        }
    }

    // Output results
    std::cout << "\n=== SIMULATION RESULTS ===" << std::endl;
    std::cout << "Total measurement period: " << MEASUREMENT_STEPS << " steps"
              << std::endl;
    std::cout << "Flux q_A (Lane A): " << flux_counters.q_A << " vehicles"
              << std::endl;
    std::cout << "Flux q_B (Lane B): " << flux_counters.q_B << " vehicles"
              << std::endl;
    std::cout << "Flux q_C (Lane C): " << flux_counters.q_C << " vehicles"
              << std::endl;
    std::cout << "Flux q_D (Lane D): " << flux_counters.q_D << " vehicles"
              << std::endl;

    int q_total = flux_counters.q_A + flux_counters.q_B + flux_counters.q_C;
    std::cout << "Total intersection capacity: " << q_total << " vehicles"
              << std::endl;

    // Calculate rates
    double rate_A = static_cast<double>(flux_counters.q_A) / MEASUREMENT_STEPS;
    double rate_B = static_cast<double>(flux_counters.q_B) / MEASUREMENT_STEPS;
    double rate_C = static_cast<double>(flux_counters.q_C) / MEASUREMENT_STEPS;

    std::cout << "\nFlux rates (vehicles/step):" << std::endl;
    std::cout << "  Lane A: " << rate_A << std::endl;
    std::cout << "  Lane B: " << rate_B << std::endl;
    std::cout << "  Lane C: " << rate_C << std::endl;

    if (win) delete win;

    return 0;
}
