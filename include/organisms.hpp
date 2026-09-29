#pragma once

#include "types.hpp"

#include <cstddef>

struct Traits {
    double attractiveness = 1.0;
    double vision = 1.0;
    double speed = 1.0;
    double size = 1.0;
    double metabolism = 1.0;
};

struct AnimalState {
    std::size_t id = 0;
    Vec2 position{};
    int age = 0;
    Sex sex = Sex::Male;
    Traits traits{};
    double hunger = 0.0;
    double thirst = 0.0;
    int action_cooldown = 0;
    int fertility_cooldown = 0;
    int max_age = 100;
    bool alive = true;
};

struct Herbivore {
    AnimalState state;
};

struct Predator {
    AnimalState state;
};

inline double hunger_capacity(const Herbivore& h) {
    return 100.0 + h.state.traits.size * 40.0;
}

inline double thirst_capacity(const Herbivore& h) {
    return hunger_capacity(h);
}

inline double hunger_capacity(const Predator& p) {
    return 200.0 + p.state.traits.size * 30.0;
}

inline double thirst_capacity(const Predator& p) {
    return hunger_capacity(p);
}

inline double herbivore_energy_ratio(const Herbivore& h) {
    return clamp01(h.state.hunger / hunger_capacity(h));
}

inline double herbivore_thirst_ratio(const Herbivore& h) {
    return clamp01(h.state.thirst / thirst_capacity(h));
}

inline double predator_energy_ratio(const Predator& p) {
    return clamp01(p.state.hunger / hunger_capacity(p));
}

inline double predator_thirst_ratio(const Predator& p) {
    return clamp01(p.state.thirst / thirst_capacity(p));
}
