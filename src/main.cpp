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

    Simulation sim;

    sim.reset();
    sim.spawn_cars(density, ratio);  // get here from command line args

    // Warmup 1000 steps
    for (double step = 0; step < WARMUP_STEPS; step += DELTA) {
        sim.step(step, density, false);
    }

    for (double step = 0; step < MAX_TIME_STEP; step += DELTA) {
        sim.step(step, density, true);

        if (visualize) {
            sim.draw(grid);
            CImg<unsigned char> zoomed =
                grid.get_resize(WIN_W, WIN_H, -100, -100, 1);

            std::string iter_text = "Step: " + std::to_string(step);
            const unsigned char white[] = {255, 255, 255};
            zoomed.draw_text(WIN_W - 200, 10, iter_text.c_str(), white, 0, 1,
                             24);

            win.display(zoomed);

            if (win.is_closed()) break;
            if (win.resize()) {
                win.resize(WIN_W, WIN_H);
            }

            win.wait(1000);  // 1 second real-time delay for visualization
        }
    }

    sim.get_stats().print_csv();
    return 0;
}
