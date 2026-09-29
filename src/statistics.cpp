#include "statistics.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace {

template <typename Container, typename Selector>
double average_trait(const Container& animals, Selector selector) {
    if (animals.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& animal : animals) {
        if (animal.state.alive) sum += selector(animal.state.traits);
    }
    std::size_t alive = 0;
    for (const auto& animal : animals) alive += animal.state.alive ? 1U : 0U;
    return alive == 0 ? 0.0 : sum / static_cast<double>(alive);
}

template <typename Container>
std::size_t alive_count(const Container& animals) {
    std::size_t count = 0;
    for (const auto& animal : animals) count += animal.state.alive ? 1U : 0U;
    return count;
}

} // namespace

void StatisticsCollector::sample(std::size_t tick, const std::vector<Herbivore>& herbivores,
                                 const std::vector<Predator>& predators) {
    TickStats s;
    s.tick = tick;
    s.herbivores = alive_count(herbivores);
    s.predators = alive_count(predators);

    s.herbivore_avg_attractiveness = average_trait(herbivores, [](const Traits& t) { return t.attractiveness; });
    s.herbivore_avg_vision = average_trait(herbivores, [](const Traits& t) { return t.vision; });
    s.herbivore_avg_speed = average_trait(herbivores, [](const Traits& t) { return t.speed; });
    s.herbivore_avg_size = average_trait(herbivores, [](const Traits& t) { return t.size; });
    s.herbivore_avg_metabolism = average_trait(herbivores, [](const Traits& t) { return t.metabolism; });

    s.predator_avg_attractiveness = average_trait(predators, [](const Traits& t) { return t.attractiveness; });
    s.predator_avg_vision = average_trait(predators, [](const Traits& t) { return t.vision; });
    s.predator_avg_speed = average_trait(predators, [](const Traits& t) { return t.speed; });
    s.predator_avg_size = average_trait(predators, [](const Traits& t) { return t.size; });
    s.predator_avg_metabolism = average_trait(predators, [](const Traits& t) { return t.metabolism; });

    samples_.push_back(s);
}

void StatisticsCollector::write_csv(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("No s'ha pogut obrir el CSV: " + filename);

    out << "Tick,Herbivores,Predators,"
           "HerbivoreAvgAttractiveness,HerbivoreAvgVision,HerbivoreAvgSpeed,HerbivoreAvgSize,HerbivoreAvgMetabolism,"
           "PredatorAvgAttractiveness,PredatorAvgVision,PredatorAvgSpeed,PredatorAvgSize,PredatorAvgMetabolism\n";

    out << std::setprecision(6);
    for (const auto& s : samples_) {
        out << s.tick << ',' << s.herbivores << ',' << s.predators << ','
            << s.herbivore_avg_attractiveness << ',' << s.herbivore_avg_vision << ','
            << s.herbivore_avg_speed << ',' << s.herbivore_avg_size << ',' << s.herbivore_avg_metabolism << ','
            << s.predator_avg_attractiveness << ',' << s.predator_avg_vision << ','
            << s.predator_avg_speed << ',' << s.predator_avg_size << ',' << s.predator_avg_metabolism << '\n';
    }
}
