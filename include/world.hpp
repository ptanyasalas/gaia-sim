#pragma once

#include "config.hpp"
#include "organisms.hpp"

#include <optional>
#include <random>
#include <vector>

struct WaterSource {
    Vec2 position{};
};

struct FoodPatch {
    Vec2 position{};
    int capacity = 0;
    int respawn_ticks = 0;
};

class World {
public:
    World(double size, std::mt19937& rng, std::size_t water_count, std::size_t food_count, int food_capacity,
          int food_respawn_ticks);

    double size() const { return size_; }
    const std::vector<WaterSource>& water_sources() const { return water_; }
    std::vector<WaterSource>& water_sources() { return water_; }
    const std::vector<FoodPatch>& food_patches() const { return food_; }
    std::vector<FoodPatch>& food_patches() { return food_; }

    std::optional<std::size_t> nearest_water(const Vec2& position, double max_distance) const;
    std::optional<std::size_t> nearest_food(const Vec2& position, double max_distance) const;
    std::optional<std::size_t> nearest_predator(const Vec2& position, const std::vector<Predator>& predators,
                                                double max_distance) const;
    std::optional<std::size_t> nearest_herbivore(const Vec2& position, const std::vector<Herbivore>& herbivores,
                                                 double max_distance) const;

    void wrap(Vec2& position) const;
    void regenerate_food();

private:
    double size_ = 100.0;
    std::vector<WaterSource> water_;
    std::vector<FoodPatch> food_;
    int food_capacity_ = 8;
    int food_respawn_ticks_ = 40;
};
