#include "simulation.hpp"

#include <cassert>

int main() {
    SimulationConfig config;
    config.ticks = 250;
    config.initial_herbivores = 30;
    config.initial_predators = 5;
    config.food_sources = 15;
    config.water_sources = 8;
    config.seed = 7;

    Simulation sim(config);
    sim.run();

    assert(sim.statistics().data().size() >= 1);
    return 0;
}
