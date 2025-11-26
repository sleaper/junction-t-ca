#include "Simulation.h"

#include <stdlib.h>

#include <algorithm>
#include <iostream>

#include "config.h"

Simulation::Simulation() { init_lanes(); }

void Simulation::reset() {
    lanes_.clear();
    cars_.clear();
    next_car_id_ = 0;
    init_lanes();
    stats_ = Statistics();
}

void Simulation::init_lanes() {
    int y_mid = SCREEN_CELLS_Y / 2;

    auto left = std::make_unique<Lane>(LaneType::Left, 0, y_mid);
    auto right = std::make_unique<Lane>(LaneType::Right, 0, y_mid + 1);

    lanes_.push_back(std::move(left));
    lanes_.push_back(std::move(right));
}

Lane* Simulation::get_lane(LaneType type) {
    return lanes_.at(static_cast<size_t>(type)).get();
}

LaneType Simulation::opposite_lane(LaneType type) {
    return type == LaneType::Left ? LaneType::Right : LaneType::Left;
}

void Simulation::draw(CImg<unsigned char>& img) {
    img.fill(0);

    for (auto& lane : lanes_) {
        lane->draw(img);
    }
}

int Simulation::front_gap(const Car* car, const Lane* lane) {
    int gap = 0;
    int car_pos = car->poss;

    for (int i = 1; i <= MAIN_LANE_LENGTH; i++) {
        int cell = (car_pos + i) % MAIN_LANE_LENGTH;  // wrap around
        if (lane->occ.at(cell) != EMPTY_CELL && lane->occ.at(cell) != car->id) {
            return gap;
        }
        gap++;
    }

    return gap;
}

int Simulation::back_gap(const Car* car, const Lane* lane) {
    int gap = 0;
    int car_pos = car->poss;

    for (int i = 1; i <= MAIN_LANE_LENGTH; i++) {
        int cell =
            (car_pos - i + MAIN_LANE_LENGTH) % MAIN_LANE_LENGTH;  // wrap around
        if (lane->occ.at(cell) != EMPTY_CELL && lane->occ.at(cell) != car->id) {
            return gap;
        }
        gap++;
    }

    return gap;
}

void Simulation::spawn_cars(double density, double aggressive_ratio) {
    int n_cars = static_cast<int>(density * MAIN_LANE_LENGTH);

    for (size_t i = 0; i < lanes_.size(); i++) {
        Lane* lane = lanes_.at(i).get();
        int spawned = 0;

        int rand_pos = rand() % (MAIN_LANE_LENGTH);

        int attempts = 0;
        const int max_attempts = MAIN_LANE_LENGTH * 10;
        while (spawned < n_cars && attempts < max_attempts) {
            rand_pos = rand() % (MAIN_LANE_LENGTH);
            attempts++;

            // Check if position is free
            if (lane->occ.at(rand_pos) != EMPTY_CELL) continue;

            auto car = std::make_unique<Car>();
            car->id = static_cast<int>(next_car_id_++);
            car->lane_id = lane->id;
            car->poss = rand_pos;
            car->aggressive =
                ((double)rand() / RAND_MAX) < aggressive_ratio ? true : false;
            car->v = 0;

            lane->occ.at(car->poss) = car->id;
            cars_.push_back(std::move(car));
            spawned++;
        }
    }
}

void Simulation::step(double mt, double density, bool collect_stats) {
    std::vector<int> next_v(cars_.size());
    std::vector<int> next_pos(cars_.size());
    std::vector<LaneType> next_lane(cars_.size());
    size_t lane_changes_this_step = 0;

    // Init next_lane vector with curr lane for each car
    for (size_t i = 0; i < cars_.size(); i++) {
        next_lane[i] = cars_[i]->lane_id;
    }

    for (size_t i = 0; i < cars_.size(); i++) {
        Car* car = cars_[i].get();
        Lane* lane = get_lane(car->lane_id);
        int v_plan = car->v;

        // Lane change
        int look_ahead = v_plan + 1;
        int look_other_ahead = LOOK_AHEAD;
        int look_other_back = LOOK_OTHER_BACK;

        Lane* other_lane = get_lane(opposite_lane(car->lane_id));

        // T1 somebody in my way
        bool incentive;
        if (MODE == MODE_TYPE::SYMMETRIC) {
            incentive = front_gap(car, lane) < look_ahead;
        } else {
            incentive = (car->lane_id == LaneType::Right)
                            ? front_gap(car, lane) < look_ahead
                            : true;
        }

        // T2 is other lane better?
        bool improvement = front_gap(car, other_lane) > look_other_ahead;

        // T3 is it safe?
        if (car->aggressive) {
            look_other_back = 1;
        }
        bool safety = back_gap(car, other_lane) > look_other_back;

        if (incentive && improvement && safety) {
            if (((double)rand() / RAND_MAX) < (LANE_CHANGE_PROB)) {
                next_lane[i] = other_lane->id;
                lane = other_lane;
                lane_changes_this_step++;
            }
        }

        // Movement in lane
        // S1: Acceleration
        v_plan = std::min(car->v + 1, VMAX);

        // S2: Deceleration due to other cars
        v_plan = std::min(v_plan, front_gap(car, lane));

        // S3: Randomization
        if (v_plan > 0 && ((double)rand() / RAND_MAX) < BREAKING_PROB) {
            v_plan = v_plan - 1;
        }

        next_v[i] = v_plan;
        next_pos[i] = car->poss + v_plan;
    }

    // Prepare for next step
    for (auto& lane : lanes_) {
        lane->clear_next();
    }

    // Commit updates to all cars
    for (size_t i = 0; i < cars_.size(); i++) {
        cars_[i]->v = next_v[i];
        cars_[i]->poss = next_pos[i] % MAIN_LANE_LENGTH;
        cars_[i]->lane_id = next_lane[i];

        Lane* lane = get_lane(cars_[i]->lane_id);
        lane->next_occ.at(cars_[i]->poss) = cars_[i]->id;
    }

    for (auto& lane : lanes_) {
        lane->swap_buffers();
    }

    if (collect_stats)
        stats_.record_step(static_cast<double>(mt), cars_, lanes_,
                           lane_changes_this_step);
}
