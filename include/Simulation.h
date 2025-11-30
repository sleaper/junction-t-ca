#ifndef SIMULATION_H
#define SIMULATION_H

#include <memory>
#include <vector>

#include "Car.h"
#include "Lane.h"
#include "Statistics.h"

class Simulation {
   public:
    Simulation();
    void reset();
    void spawn_cars(double density, double aggressive_ratio);
    void step(double model_time, bool collect_stats, bool asymmetric);
    static LaneType opposite_lane(LaneType type);
    const Statistics& get_stats() const { return stats_; }

   private:
    std::vector<std::unique_ptr<Lane>> lanes_;
    std::vector<std::unique_ptr<Car>> cars_;
    size_t next_car_id_ = 0;
    Statistics stats_;

    void init_lanes();
    Lane* get_lane(LaneType type);
};

#endif  // SIMULATION_H
