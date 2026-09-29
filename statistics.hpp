#pragma once

#include "organisms.hpp"

#include <cstddef>
#include <string>
#include <vector>

struct TickStats {
    std::size_t tick = 0;
    std::size_t herbivores = 0;
    std::size_t predators = 0;
    double herbivore_avg_attractiveness = 0.0;
    double herbivore_avg_vision = 0.0;
    double herbivore_avg_speed = 0.0;
    double herbivore_avg_size = 0.0;
    double herbivore_avg_metabolism = 0.0;
    double predator_avg_attractiveness = 0.0;
    double predator_avg_vision = 0.0;
    double predator_avg_speed = 0.0;
    double predator_avg_size = 0.0;
    double predator_avg_metabolism = 0.0;
};

class StatisticsCollector {
public:
    void sample(std::size_t tick, const std::vector<Herbivore>& herbivores,
                const std::vector<Predator>& predators);
    void write_csv(const std::string& filename) const;
    const std::vector<TickStats>& data() const { return samples_; }

private:
    std::vector<TickStats> samples_;
};
