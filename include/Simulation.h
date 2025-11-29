#ifndef SIMULATION_H
#define SIMULATION_H

#include <memory>
#include <vector>

#include "CImg.h"
#include "Car.h"
#include "Lane.h"
#include "Statistics.h"

using namespace cimg_library;

class Simulation {
   public:
    Simulation();
    void reset();
    void spawn_cars(double density, double aggressive_ratio);
    void step(double model_time, bool collect_stats, bool asymmetric);
    void draw(CImg<unsigned char>& img);

    const Statistics& get_stats() const { return stats_; }

   private:
    std::vector<std::unique_ptr<Lane>> lanes_;
    std::vector<std::unique_ptr<Car>> cars_;
    size_t next_car_id_ = 0;
    Statistics stats_;

    void init_lanes();
    Lane* get_lane(LaneType type);
    static LaneType opposite_lane(LaneType type);
    int front_gap(const Car* car, const Lane* lane);
    int back_gap(const Car* car, const Lane* lane);
};

#endif  // SIMULATION_H
