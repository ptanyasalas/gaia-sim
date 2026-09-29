#pragma once

#include "config.hpp"
#include "statistics.hpp"
#include "world.hpp"

#include <cstddef>
#include <random>
#include <string>
#include <vector>

class Simulation {
public:
    explicit Simulation(const SimulationConfig& config);

    void run();
    void step();
    void write_csv(const std::string& filename) const;

    const std::vector<Herbivore>& herbivores() const { return herbivores_; }
    const std::vector<Predator>& predators() const { return predators_; }
    const StatisticsCollector& statistics() const { return statistics_; }

private:
    SimulationConfig config_;
    std::mt19937 rng_;
    World world_;
    std::vector<Herbivore> herbivores_;
    std::vector<Predator> predators_;
    StatisticsCollector statistics_;
    std::size_t tick_ = 0;
    std::size_t next_id_ = 1;

    void initialize_populations();
    void age_and_needs();
    void predator_turn();
    void herbivore_turn();
    void reproduce_herbivores();
    void reproduce_predators();
    void cleanup_dead();

    void move_predator(Predator& predator);
    void move_herbivore(Herbivore& herbivore);

    Herbivore make_herbivore(const Vec2& position, const Traits& traits, Sex sex, int age = 0);
    Predator make_predator(const Vec2& position, const Traits& traits, Sex sex, int age = 0);

    Traits child_herbivore_traits(const Traits& a, const Traits& b);
    Traits child_predator_traits(const Traits& a, const Traits& b);

    std::size_t herbivore_capacity() const;
    std::size_t predator_capacity() const;
    double density_factor(std::size_t population, std::size_t capacity) const;
};
