#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "Simulation.h"
#include "config.h"

void print_usage(const char* program_name) {
    std::cerr
        << "Usage: " << program_name << " [options]\n"
        << "Options:\n"
        << "  -d <density>       Set traffic density (default: 0.1)\n"
        << "  -a <percent>       Set percentage of aggressive drivers "
           "(default: 0.0)\n"
        << "  -w <steps>         Set number of warmup steps (default: "
        << WARMUP_STEPS << ")\n"
        << "  -s <steps>         Set number of simulation steps (default: "
        << MAX_TIME_STEP << ")\n"
        << "  -p                 Print CSV output\n"
        << "  -v                 Enable visualization\n"
        << "  -h                 Show this help message\n";
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));

    double density = 0.1;
    double aggressive_ratio = 0.0;
    int warmup_steps = WARMUP_STEPS;
    int simulation_steps = MAX_TIME_STEP;
    bool asymmetric = false;
    bool visualize = false;
    bool print_csv = false;

    int opt;
    while ((opt = getopt(argc, argv, "d:a:w:s:pvyh")) != -1) {
        switch (opt) {
            case 'd':
                try {
                    density = std::stod(optarg);
                    if (density < 0.0 || density > 1.0) {
                        std::cerr
                            << "Error: Density must be between 0 and 1.0\n";
                        return 1;
                    }
                } catch (...) {
                    std::cerr << "Error: Invalid number for density\n";
                    return 1;
                }
                break;
            case 'a':
                try {
                    aggressive_ratio = std::stod(optarg);
                    if (aggressive_ratio < 0.0 || aggressive_ratio > 1.0) {
                        std::cerr
                            << "Error: Aggressive driver percentage must be "
                               "between 0 and 1.0\n";
                        return 1;
                    }
                } catch (...) {
                    std::cerr << "Error: Invalid number for aggressive ratio\n";
                    return 1;
                }
                break;
            case 'w':
                try {
                    warmup_steps = std::stoi(optarg);
                    if (warmup_steps < 0) {
                        std::cerr
                            << "Error: Warmup steps must be non-negative\n";
                        return 1;
                    }
                } catch (...) {
                    std::cerr << "Error: Invalid number for warmup steps\n";
                    return 1;
                }
                break;
            case 's':
                try {
                    simulation_steps = std::stoi(optarg);
                    if (simulation_steps < 0) {
                        std::cerr
                            << "Error: Simulation steps must be non-negative\n";
                        return 1;
                    }
                } catch (...) {
                    std::cerr << "Error: Invalid number for simulation steps\n";
                    return 1;
                }
                break;
            case 'v':
                visualize = true;
                break;
            case 'p':
                print_csv = true;
                break;
            case 'y':
                asymmetric = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    Simulation sim;

    sim.reset();
    sim.spawn_cars(density, aggressive_ratio);

    // Warmup steps
    for (double step = 0; step < warmup_steps; step += DELTA) {
        sim.step(step, false, asymmetric);
    }

    for (double step = 0; step < simulation_steps; step += DELTA) {
        sim.step(step, true, asymmetric);
    }

    if (print_csv) {
        sim.get_stats().print_csv();
        sim.get_stats().dump_space_time("space_time.csv");
    } else {
        sim.get_stats().print_summary();
    }
    return 0;
}
