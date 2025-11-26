#ifndef STATISTICS_H
#define STATISTICS_H

#include <memory>
#include <string>
#include <vector>

#include "Car.h"
#include "Lane.h"

struct StepMetrics {
    double model_time = 0.0;
    double density = 0.0;
    double aggressive = 0.0;
    double flow = 0.0;
    double lane_change_rate = 0.0;  // lane changes per cell per step
    double left_flow = 0.0;
    double right_flow = 0.0;
    std::vector<std::vector<int>> lanes;
};

class Statistics {
   public:
    void record_step(double model_time,
                     const std::vector<std::unique_ptr<Car>>& cars,
                     const std::vector<std::unique_ptr<Lane>>& lanes,
                     size_t lane_changes);

    void dump_space_time(const std::string& path) const;
    void print_csv() const;
    void print_summary() const;

   private:
    std::vector<StepMetrics> samples_;
    double density_accum_ = 0.0;
    double aggressive_accum_ = 0.0;
    double flow_accum_ = 0.0;
    double avg_speed_accum_ = 0.0;
    double lane_change_accum_ = 0.0;
    double left_flow_accum_ = 0.0;
    double right_flow_accum_ = 0.0;
};

#endif  // STATISTICS_H
