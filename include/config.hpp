#pragma once

#include <cstdint>
#include <cstddef>

struct SimulationConfig {
    double world_size = 100.0;
    std::size_t water_sources = 16;
    std::size_t food_sources = 40;
    std::size_t initial_herbivores = 80;
    std::size_t initial_predators = 12;
    std::uint64_t ticks = 5000;
    std::uint32_t seed = 42;

    // Simulation cadence: keeps the model deterministic and independent of wall-clock speed.
    int age_every = 10;
    int needs_every = 6;
    int stats_every = 20;

    // Resource economy.
    int food_patch_capacity = 12;
    int food_respawn_ticks = 30;
    double food_energy = 34.0;
    double water_gain = 90.0;

    // Movement / perception.
    double herbivore_water_search = 10.0;
    double herbivore_food_search = 14.0;
    double herbivore_predator_search = 10.0;
    double predator_prey_search = 18.0;
    double predator_water_search = 15.0;
    double resource_contact_distance = 2.0;
    double predator_contact_distance = 2.5;

    // Reproduction.
    int herbivore_maturity = 12;
    int predator_maturity = 10;
    double herbivore_mating_radius = 4.0;
    double predator_mating_radius = 9.0;
    double herbivore_base_birth_chance = 0.16;
    double predator_base_birth_chance = 0.16;
    int herbivore_fertility_cooldown = 32;
    int predator_fertility_cooldown = 30;
    double herbivore_birth_cost = 18.0;
    double predator_birth_cost = 35.0;

    // Soft carrying capacities. Auto mode makes the limits depend on resources.
    bool auto_capacity = true;
    std::size_t herbivore_soft_capacity = 150;
    std::size_t predator_soft_capacity = 30;

    // Genetics. Traits are NOT re-scaled on construction: this fixes the original speed decay bug.
    double mutation_probability = 0.08;
    double mutation_sigma = 0.22;

    // Trait ranges.
    double herbivore_attractiveness_min = 1.0;
    double herbivore_attractiveness_max = 10.0;
    double herbivore_vision_min = 1.0;
    double herbivore_vision_max = 5.0;
    double herbivore_speed_min = 1.0;
    double herbivore_speed_max = 5.0;
    double herbivore_size_min = 1.0;
    double herbivore_size_max = 5.0;
    double herbivore_metabolism_min = 1.0;
    double herbivore_metabolism_max = 5.0;

    double predator_attractiveness_min = 1.0;
    double predator_attractiveness_max = 10.0;
    double predator_vision_min = 2.0;
    double predator_vision_max = 5.0;
    double predator_speed_min = 2.0;
    double predator_speed_max = 5.0;
    double predator_size_min = 3.0;
    double predator_size_max = 5.0;
    double predator_metabolism_min = 2.0;
    double predator_metabolism_max = 5.0;
};
