#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

namespace {

double mean(double a, double b) { return (a + b) * 0.5; }

template <typename T>
void erase_dead(std::vector<T>& animals) {
    animals.erase(std::remove_if(animals.begin(), animals.end(),
                                 [](const T& a) { return !a.state.alive; }),
                  animals.end());
}

} // namespace

Simulation::Simulation(const SimulationConfig& config)
    : config_(config),
      rng_(config.seed),
      world_(config.world_size, rng_, config.water_sources, config.food_sources,
             config.food_patch_capacity, config.food_respawn_ticks) {
    initialize_populations();
    statistics_.sample(0, herbivores_, predators_);
}

void Simulation::initialize_populations() {
    herbivores_.reserve(config_.initial_herbivores * 2);
    predators_.reserve(config_.initial_predators * 2);

    for (std::size_t i = 0; i < config_.initial_herbivores; ++i) {
        Traits t{
            random_real(rng_, config_.herbivore_attractiveness_min, config_.herbivore_attractiveness_max),
            random_real(rng_, config_.herbivore_vision_min, config_.herbivore_vision_max),
            random_real(rng_, config_.herbivore_speed_min, config_.herbivore_speed_max),
            random_real(rng_, config_.herbivore_size_min, config_.herbivore_size_max),
            random_real(rng_, config_.herbivore_metabolism_min, config_.herbivore_metabolism_max)
        };
        herbivores_.push_back(make_herbivore(
            {random_real(rng_, 0.0, config_.world_size), random_real(rng_, 0.0, config_.world_size)},
            t, random_sex(rng_), random_int(rng_, 0, 20)));
    }

    for (std::size_t i = 0; i < config_.initial_predators; ++i) {
        Traits t{
            random_real(rng_, config_.predator_attractiveness_min, config_.predator_attractiveness_max),
            random_real(rng_, config_.predator_vision_min, config_.predator_vision_max),
            random_real(rng_, config_.predator_speed_min, config_.predator_speed_max),
            random_real(rng_, config_.predator_size_min, config_.predator_size_max),
            random_real(rng_, config_.predator_metabolism_min, config_.predator_metabolism_max)
        };
        predators_.push_back(make_predator(
            {random_real(rng_, 0.0, config_.world_size), random_real(rng_, 0.0, config_.world_size)},
            t, random_sex(rng_), random_int(rng_, 0, 10)));
    }
}

Herbivore Simulation::make_herbivore(const Vec2& position, const Traits& traits, Sex sex, int age) {
    Herbivore h;
    h.state.id = next_id_++;
    h.state.position = position;
    h.state.age = age;
    h.state.sex = sex;
    h.state.traits = traits;
    h.state.max_age = random_int(rng_, 90, 130);
    const double cap = hunger_capacity(h);
    h.state.hunger = cap;
    h.state.thirst = cap;
    h.state.action_cooldown = 0;
    h.state.fertility_cooldown = 0;
    return h;
}

Predator Simulation::make_predator(const Vec2& position, const Traits& traits, Sex sex, int age) {
    Predator p;
    p.state.id = next_id_++;
    p.state.position = position;
    p.state.age = age;
    p.state.sex = sex;
    p.state.traits = traits;
    p.state.max_age = random_int(rng_, 70, 100);
    const double cap = hunger_capacity(p);
    p.state.hunger = cap;
    p.state.thirst = cap;
    p.state.action_cooldown = 0;
    p.state.fertility_cooldown = 0;
    return p;
}

std::size_t Simulation::herbivore_capacity() const {
    if (!config_.auto_capacity) return config_.herbivore_soft_capacity;
    // Food is the main limiting resource for herbivores.
    return std::max<std::size_t>(20, config_.food_sources * 5 + config_.water_sources * 2);
}

std::size_t Simulation::predator_capacity() const {
    if (!config_.auto_capacity) return config_.predator_soft_capacity;
    return std::max<std::size_t>(4, herbivore_capacity() / 6);
}

double Simulation::density_factor(std::size_t population, std::size_t capacity) const {
    if (capacity == 0 || population >= capacity) return 0.0;
    const double free_space = 1.0 - static_cast<double>(population) / static_cast<double>(capacity);
    return std::pow(std::max(0.0, free_space), 1.35);
}

void Simulation::age_and_needs() {
    for (auto& h : herbivores_) {
        if (!h.state.alive) continue;
        if (tick_ % static_cast<std::size_t>(config_.age_every) == 0) ++h.state.age;
        if (tick_ % static_cast<std::size_t>(config_.needs_every) == 0) {
            const double drain = 1.0 + h.state.traits.metabolism * 0.24;
            h.state.hunger -= drain;
            h.state.thirst -= drain;
        }
        if (h.state.action_cooldown > 0) --h.state.action_cooldown;
        if (h.state.fertility_cooldown > 0) --h.state.fertility_cooldown;
        if (h.state.hunger <= 0.0 || h.state.thirst <= 0.0 || h.state.age > h.state.max_age) h.state.alive = false;
    }

    for (auto& p : predators_) {
        if (!p.state.alive) continue;
        if (tick_ % static_cast<std::size_t>(config_.age_every) == 0) ++p.state.age;
        if (tick_ % static_cast<std::size_t>(config_.needs_every) == 0) {
            const double hunger_drain = 0.75 + p.state.traits.metabolism * 0.18;
            const double thirst_drain = 0.40 + p.state.traits.metabolism * 0.12;
            p.state.hunger -= hunger_drain;
            p.state.thirst -= thirst_drain;
        }
        if (p.state.action_cooldown > 0) --p.state.action_cooldown;
        if (p.state.fertility_cooldown > 0) --p.state.fertility_cooldown;
        if (p.state.hunger <= 0.0 || p.state.thirst <= 0.0 || p.state.age > p.state.max_age) p.state.alive = false;
    }
}

void Simulation::move_predator(Predator& predator) {
    auto& p = predator.state;
    const double energy_ratio = predator_energy_ratio(predator);

    // Predators hunt primarily when hungry. This prevents the predator population
    // from treating every encountered herbivore as an unconditional kill target.
    if (energy_ratio < 0.80) {
        const auto prey = world_.nearest_herbivore(p.position, herbivores_,
                                                   p.traits.vision * config_.predator_prey_search);
        if (prey) {
            move_along(p.position, toroidal_delta(p.position, herbivores_[*prey].state.position, world_.size()),
                       p.traits.speed, world_.size());
            if (distance_sq(p.position, herbivores_[*prey].state.position, world_.size()) <=
                config_.predator_contact_distance * config_.predator_contact_distance) {
                auto& target = herbivores_[*prey];
                if (target.state.alive) {
                    target.state.alive = false;
                    p.hunger = std::min(hunger_capacity(predator),
                                         p.hunger + 100.0 + target.state.traits.size * 24.0);
                }
            }
            return;
        }
    }

    const auto water = world_.nearest_water(p.position, p.traits.vision * config_.predator_water_search);
    if (predator_thirst_ratio(predator) < 0.75 && water) {
        move_along(p.position, toroidal_delta(p.position, world_.water_sources()[*water].position, world_.size()),
                   p.traits.speed, world_.size());
    } else {
        const double angle = random_real(rng_, 0.0, 2.0 * 3.14159265358979323846);
        move_along(p.position, {std::cos(angle), std::sin(angle)}, p.traits.speed * 0.75, world_.size());
    }
}

void Simulation::move_herbivore(Herbivore& herbivore) {
    auto& h = herbivore.state;

    const auto predator = world_.nearest_predator(
        h.position, predators_, h.traits.vision * config_.herbivore_predator_search);
    if (predator) {
        const Vec2 away = toroidal_delta(predators_[*predator].state.position, h.position, world_.size());
        move_along(h.position, away, h.traits.speed, world_.size());
        return;
    }

    const auto water = world_.nearest_water(h.position, h.traits.vision * config_.herbivore_water_search);
    const auto food = world_.nearest_food(h.position, h.traits.vision * config_.herbivore_food_search);

    if (h.thirst < h.hunger && water) {
        move_along(h.position, toroidal_delta(h.position, world_.water_sources()[*water].position, world_.size()),
                   h.traits.speed, world_.size());
    } else if (food) {
        move_along(h.position, toroidal_delta(h.position, world_.food_patches()[*food].position, world_.size()),
                   h.traits.speed, world_.size());
    } else if (water) {
        move_along(h.position, toroidal_delta(h.position, world_.water_sources()[*water].position, world_.size()),
                   h.traits.speed, world_.size());
    } else {
        const double angle = random_real(rng_, 0.0, 2.0 * 3.14159265358979323846);
        move_along(h.position, {std::cos(angle), std::sin(angle)}, h.traits.speed, world_.size());
    }
}

void Simulation::predator_turn() {
    for (auto& p : predators_) {
        if (!p.state.alive || p.state.action_cooldown > 0) continue;

        const bool at_water = [&]() {
            const auto water = world_.nearest_water(p.state.position, config_.resource_contact_distance);
            if (!water) return false;
            if (p.state.thirst < thirst_capacity(p)) {
                p.state.thirst = std::min(thirst_capacity(p), p.state.thirst + config_.water_gain);
                return true;
            }
            return false;
        }();

        if (!at_water) move_predator(p);
        p.state.action_cooldown = std::max(1, 4 - static_cast<int>(std::round(p.state.traits.speed * 0.4)));
    }
}

void Simulation::herbivore_turn() {
    for (auto& h : herbivores_) {
        if (!h.state.alive || h.state.action_cooldown > 0) continue;

        bool resource_used = false;
        if (const auto water = world_.nearest_water(h.state.position, config_.resource_contact_distance)) {
            if (h.state.thirst < thirst_capacity(h)) {
                h.state.thirst = std::min(thirst_capacity(h), h.state.thirst + config_.water_gain);
                resource_used = true;
            }
        }

        if (!resource_used) {
            if (const auto food = world_.nearest_food(h.state.position, config_.resource_contact_distance)) {
                auto& patch = world_.food_patches()[*food];
                if (patch.capacity > 0 && h.state.hunger < hunger_capacity(h)) {
                    h.state.hunger = std::min(hunger_capacity(h), h.state.hunger + config_.food_energy + h.state.traits.size * 2.0);
                    --patch.capacity;
                    if (patch.capacity == 0) patch.respawn_ticks = config_.food_respawn_ticks;
                    resource_used = true;
                }
            }
        }

        if (!resource_used) move_herbivore(h);
        h.state.action_cooldown = std::max(1, 4 - static_cast<int>(std::round(h.state.traits.speed * 0.45)));
    }
}

Traits Simulation::child_herbivore_traits(const Traits& a, const Traits& b) {
    return {
        normal_mutation(mean(a.attractiveness, b.attractiveness), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.herbivore_attractiveness_min, config_.herbivore_attractiveness_max),
        normal_mutation(mean(a.vision, b.vision), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.herbivore_vision_min, config_.herbivore_vision_max),
        normal_mutation(mean(a.speed, b.speed), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.herbivore_speed_min, config_.herbivore_speed_max),
        normal_mutation(mean(a.size, b.size), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.herbivore_size_min, config_.herbivore_size_max),
        normal_mutation(mean(a.metabolism, b.metabolism), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.herbivore_metabolism_min, config_.herbivore_metabolism_max)
    };
}

Traits Simulation::child_predator_traits(const Traits& a, const Traits& b) {
    return {
        normal_mutation(mean(a.attractiveness, b.attractiveness), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.predator_attractiveness_min, config_.predator_attractiveness_max),
        normal_mutation(mean(a.vision, b.vision), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.predator_vision_min, config_.predator_vision_max),
        normal_mutation(mean(a.speed, b.speed), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.predator_speed_min, config_.predator_speed_max),
        normal_mutation(mean(a.size, b.size), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.predator_size_min, config_.predator_size_max),
        normal_mutation(mean(a.metabolism, b.metabolism), rng_, config_.mutation_probability,
                        config_.mutation_sigma, config_.predator_metabolism_min, config_.predator_metabolism_max)
    };
}

void Simulation::reproduce_herbivores() {
    if (herbivores_.empty()) return;
    const double density = density_factor(herbivores_.size(), herbivore_capacity());
    if (density <= 0.0) return;

    std::vector<bool> paired(herbivores_.size(), false);
    std::vector<Herbivore> newborns;
    newborns.reserve(herbivores_.size() / 10 + 1);

    for (std::size_t i = 0; i < herbivores_.size(); ++i) {
        auto& female = herbivores_[i];
        if (!female.state.alive || female.state.sex != Sex::Female || paired[i]) continue;
        if (female.state.age <= config_.herbivore_maturity || female.state.fertility_cooldown > 0) continue;
        if (herbivore_energy_ratio(female) < 0.45 || herbivore_thirst_ratio(female) < 0.45) continue;

        std::optional<std::size_t> best_male;
        double best_distance = std::numeric_limits<double>::max();
        for (std::size_t j = 0; j < herbivores_.size(); ++j) {
            auto& male = herbivores_[j];
            if (!male.state.alive || male.state.sex != Sex::Male || paired[j] || male.state.fertility_cooldown > 0) continue;
            if (male.state.age <= config_.herbivore_maturity) continue;
            if (herbivore_energy_ratio(male) < 0.45 || herbivore_thirst_ratio(male) < 0.45) continue;
            const double d2 = distance_sq(female.state.position, male.state.position, world_.size());
            if (d2 <= config_.herbivore_mating_radius * config_.herbivore_mating_radius && d2 < best_distance) {
                best_distance = d2;
                best_male = j;
            }
        }

        if (!best_male) continue;
        auto& male = herbivores_[*best_male];
        const double attraction = 0.5 + 0.5 * ((female.state.traits.attractiveness + male.state.traits.attractiveness) / 20.0);
        const double chance = config_.herbivore_base_birth_chance * density * attraction;
        if (random_real(rng_, 0.0, 1.0) > chance) continue;

        female.state.hunger -= config_.herbivore_birth_cost;
        male.state.hunger -= config_.herbivore_birth_cost * 0.65;
        female.state.fertility_cooldown = config_.herbivore_fertility_cooldown;
        male.state.fertility_cooldown = config_.herbivore_fertility_cooldown;
        paired[i] = true;
        paired[*best_male] = true;
        newborns.push_back(make_herbivore(female.state.position, child_herbivore_traits(female.state.traits, male.state.traits), random_sex(rng_)));
    }

    herbivores_.insert(herbivores_.end(), newborns.begin(), newborns.end());
}

void Simulation::reproduce_predators() {
    if (predators_.empty()) return;
    const double density = density_factor(predators_.size(), predator_capacity());
    if (density <= 0.0) return;

    std::vector<bool> paired(predators_.size(), false);
    std::vector<Predator> newborns;
    newborns.reserve(predators_.size() / 10 + 1);

    for (std::size_t i = 0; i < predators_.size(); ++i) {
        auto& female = predators_[i];
        if (!female.state.alive || female.state.sex != Sex::Female || paired[i]) continue;
        if (female.state.age <= config_.predator_maturity || female.state.fertility_cooldown > 0) continue;
        if (predator_energy_ratio(female) < 0.58 || predator_thirst_ratio(female) < 0.45) continue;

        std::optional<std::size_t> best_male;
        double best_distance = std::numeric_limits<double>::max();
        for (std::size_t j = 0; j < predators_.size(); ++j) {
            auto& male = predators_[j];
            if (!male.state.alive || male.state.sex != Sex::Male || paired[j] || male.state.fertility_cooldown > 0) continue;
            if (male.state.age <= config_.predator_maturity) continue;
            if (predator_energy_ratio(male) < 0.58 || predator_thirst_ratio(male) < 0.45) continue;
            const double d2 = distance_sq(female.state.position, male.state.position, world_.size());
            if (d2 <= config_.predator_mating_radius * config_.predator_mating_radius && d2 < best_distance) {
                best_distance = d2;
                best_male = j;
            }
        }

        if (!best_male) continue;
        auto& male = predators_[*best_male];
        const double attraction = 0.5 + 0.5 * ((female.state.traits.attractiveness + male.state.traits.attractiveness) / 20.0);
        const double chance = config_.predator_base_birth_chance * density * attraction;
        if (random_real(rng_, 0.0, 1.0) > chance) continue;

        female.state.hunger -= config_.predator_birth_cost;
        male.state.hunger -= config_.predator_birth_cost * 0.6;
        female.state.fertility_cooldown = config_.predator_fertility_cooldown;
        male.state.fertility_cooldown = config_.predator_fertility_cooldown;
        paired[i] = true;
        paired[*best_male] = true;
        newborns.push_back(make_predator(female.state.position, child_predator_traits(female.state.traits, male.state.traits), random_sex(rng_)));
    }

    predators_.insert(predators_.end(), newborns.begin(), newborns.end());
}

void Simulation::cleanup_dead() {
    erase_dead(herbivores_);
    erase_dead(predators_);
}

void Simulation::step() {
    ++tick_;
    age_and_needs();
    predator_turn();
    cleanup_dead();

    herbivore_turn();
    cleanup_dead();

    reproduce_herbivores();
    reproduce_predators();
    cleanup_dead();

    world_.regenerate_food();

    if (tick_ % static_cast<std::size_t>(config_.stats_every) == 0) {
        statistics_.sample(tick_, herbivores_, predators_);
    }
}

void Simulation::run() {
    while (tick_ < config_.ticks) {
        step();
        if (tick_ % 250 == 0 || tick_ == config_.ticks) {
            std::cout << "Tick " << tick_ << " | herbivors=" << herbivores_.size()
                      << " | depredadors=" << predators_.size() << '\n';
        }
        if (herbivores_.empty() && predators_.empty()) break;
    }
}

void Simulation::write_csv(const std::string& filename) const {
    statistics_.write_csv(filename);
}
