#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>

struct Vec2 {
    double x = 0.0;
    double y = 0.0;
};

inline double wrap_delta(double delta, double world_size) {
    if (delta > world_size * 0.5) return delta - world_size;
    if (delta < -world_size * 0.5) return delta + world_size;
    return delta;
}

inline Vec2 toroidal_delta(const Vec2& from, const Vec2& to, double world_size) {
    return {wrap_delta(to.x - from.x, world_size), wrap_delta(to.y - from.y, world_size)};
}

inline double distance_sq(const Vec2& a, const Vec2& b, double world_size) {
    const auto d = toroidal_delta(a, b, world_size);
    return d.x * d.x + d.y * d.y;
}

inline void move_along(Vec2& position, const Vec2& delta, double speed, double world_size) {
    const double length_sq = delta.x * delta.x + delta.y * delta.y;
    if (length_sq > 1e-12) {
        const double inv_length = 1.0 / std::sqrt(length_sq);
        position.x += delta.x * inv_length * speed;
        position.y += delta.y * inv_length * speed;
    }
    position.x = std::fmod(position.x, world_size);
    position.y = std::fmod(position.y, world_size);
    if (position.x < 0.0) position.x += world_size;
    if (position.y < 0.0) position.y += world_size;
}

inline double clamp01(double v) {
    return std::clamp(v, 0.0, 1.0);
}

inline double normal_mutation(double base, std::mt19937& rng, double probability, double sigma,
                              double min_value, double max_value) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    double result = base;
    if (chance(rng) < probability) {
        std::normal_distribution<double> mutation(0.0, sigma);
        result += mutation(rng);
    }
    return std::clamp(result, min_value, max_value);
}

inline int random_int(std::mt19937& rng, int low, int high) {
    std::uniform_int_distribution<int> d(low, high);
    return d(rng);
}

inline double random_real(std::mt19937& rng, double low, double high) {
    std::uniform_real_distribution<double> d(low, high);
    return d(rng);
}

enum class Sex { Male, Female };

enum class Species { Herbivore, Predator };

inline Sex random_sex(std::mt19937& rng) {
    return random_int(rng, 0, 1) == 0 ? Sex::Male : Sex::Female;
}
