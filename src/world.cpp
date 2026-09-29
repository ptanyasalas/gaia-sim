#include "world.hpp"

#include <limits>

World::World(double size, std::mt19937& rng, std::size_t water_count, std::size_t food_count,
             int food_capacity, int food_respawn_ticks)
    : size_(size), food_capacity_(food_capacity), food_respawn_ticks_(food_respawn_ticks) {
    water_.reserve(water_count);
    food_.reserve(food_count);

    for (std::size_t i = 0; i < water_count; ++i) {
        water_.push_back({{random_real(rng, 0.0, size_), random_real(rng, 0.0, size_)}});
    }

    for (std::size_t i = 0; i < food_count; ++i) {
        food_.push_back({{random_real(rng, 0.0, size_), random_real(rng, 0.0, size_)}, food_capacity_, 0});
    }
}

std::optional<std::size_t> World::nearest_water(const Vec2& position, double max_distance) const {
    const double max_sq = max_distance * max_distance;
    double best_sq = std::numeric_limits<double>::max();
    std::optional<std::size_t> best;

    for (std::size_t i = 0; i < water_.size(); ++i) {
        const double d2 = distance_sq(position, water_[i].position, size_);
        if (d2 <= max_sq && d2 < best_sq) {
            best_sq = d2;
            best = i;
        }
    }
    return best;
}

std::optional<std::size_t> World::nearest_food(const Vec2& position, double max_distance) const {
    const double max_sq = max_distance * max_distance;
    double best_sq = std::numeric_limits<double>::max();
    std::optional<std::size_t> best;

    for (std::size_t i = 0; i < food_.size(); ++i) {
        if (food_[i].capacity <= 0) continue;
        const double d2 = distance_sq(position, food_[i].position, size_);
        if (d2 <= max_sq && d2 < best_sq) {
            best_sq = d2;
            best = i;
        }
    }
    return best;
}

std::optional<std::size_t> World::nearest_predator(const Vec2& position, const std::vector<Predator>& predators,
                                                   double max_distance) const {
    const double max_sq = max_distance * max_distance;
    double best_sq = std::numeric_limits<double>::max();
    std::optional<std::size_t> best;

    for (std::size_t i = 0; i < predators.size(); ++i) {
        if (!predators[i].state.alive) continue;
        const double d2 = distance_sq(position, predators[i].state.position, size_);
        if (d2 <= max_sq && d2 < best_sq) {
            best_sq = d2;
            best = i;
        }
    }
    return best;
}

std::optional<std::size_t> World::nearest_herbivore(const Vec2& position, const std::vector<Herbivore>& herbivores,
                                                    double max_distance) const {
    const double max_sq = max_distance * max_distance;
    double best_sq = std::numeric_limits<double>::max();
    std::optional<std::size_t> best;

    for (std::size_t i = 0; i < herbivores.size(); ++i) {
        if (!herbivores[i].state.alive) continue;
        const double d2 = distance_sq(position, herbivores[i].state.position, size_);
        if (d2 <= max_sq && d2 < best_sq) {
            best_sq = d2;
            best = i;
        }
    }
    return best;
}

void World::wrap(Vec2& position) const {
    position.x = std::fmod(position.x, size_);
    position.y = std::fmod(position.y, size_);
    if (position.x < 0.0) position.x += size_;
    if (position.y < 0.0) position.y += size_;
}

void World::regenerate_food() {
    for (auto& patch : food_) {
        if (patch.capacity <= 0) {
            --patch.respawn_ticks;
            if (patch.respawn_ticks <= 0) {
                patch.capacity = food_capacity_;
                patch.respawn_ticks = 0;
            }
        }
    }
}
