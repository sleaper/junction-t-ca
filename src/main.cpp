#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>
#include <vector>

#include "CImg.h"
#include "Simulation.h"
#include "config.h"

using namespace cimg_library;

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    bool visualize = false;

    const int WIN_W = SCREEN_CELLS_X * CELL_PIXELS;
    const int WIN_H = SCREEN_CELLS_Y * CELL_PIXELS;
    CImg<unsigned char> grid(SCREEN_CELLS_X, SCREEN_CELLS_Y, 1, 3, 0);

    CImgDisplay win;

    if (visualize) win = CImgDisplay(WIN_W, WIN_H, "Simulation grid");

    std::vector<double> flow;
    std::vector<double> lane_change;
    std::vector<double> densities;
    std::vector<double> left_flow;
    std::vector<double> right_flow;

    Simulation sim;

    for (double density = 0.01; density <= 0.5; density += 0.01) {
        sim.reset();
        sim.spawn_cars(density);

        // Warmup 1000 steps
        // for (double step = 0; step < 1000; step += DELTA) {
        //     sim.step(step, density);
        // }

        for (double step = 0; step < MAX_TIME_STEP; step += DELTA) {
            sim.step(step, density);

            if (visualize) {
                sim.draw(grid);
                CImg<unsigned char> zoomed =
                    grid.get_resize(WIN_W, WIN_H, -100, -100, 1);

                std::string iter_text = "Step: " + std::to_string(step);
                const unsigned char white[] = {255, 255, 255};
                zoomed.draw_text(WIN_W - 200, 10, iter_text.c_str(), white, 0,
                                 1, 24);

                win.display(zoomed);

                if (win.is_closed()) break;
                if (win.resize()) {
                    win.resize(WIN_W, WIN_H);
                }

                win.wait(1000);  // 1 second real-time delay for visualization
            }
        }

        const auto& stats = sim.get_stats();
        densities.push_back(density);
        flow.push_back(stats.get_average_flow());
        lane_change.push_back(stats.get_average_lane_change_rate());
        left_flow.push_back(stats.get_average_left_flow());
        right_flow.push_back(stats.get_average_right_flow());
        stats.print_summary();
    }

    try {
        Statistics stats_saver;
        stats_saver.save_final_statistics("final_statistics.csv", densities,
                                          flow, lane_change, left_flow,
                                          right_flow);
        // g_stats.dump_space_time("space_time.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error dumping statistics: " << e.what() << '\n';
    }

    return 0;
}
