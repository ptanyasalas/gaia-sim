#include "simulation.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <cstdint>

namespace {

std::uint64_t parse_u64(const char* value) {
    return static_cast<std::uint64_t>(std::stoull(value));
}

double parse_double(const char* value) {
    return std::stod(value);
}

std::size_t parse_size(const char* value) {
    return static_cast<std::size_t>(std::stoull(value));
}

void validate_config(const SimulationConfig& config) {
    if (config.world_size <= 0.0) {
        throw std::invalid_argument("world_size must be greater than 0");
    }
    if (config.water_sources == 0) {
        throw std::invalid_argument("water_sources must be greater than 0");
    }
    if (config.food_sources == 0) {
        throw std::invalid_argument("food_sources must be greater than 0");
    }
    if (config.initial_herbivores > 0 && config.herbivore_soft_capacity == 0) {
        throw std::invalid_argument("herbivore_soft_capacity must be greater than 0");
    }
    if (config.initial_predators > 0 && config.predator_soft_capacity == 0) {
        throw std::invalid_argument("predator_soft_capacity must be greater than 0");
    }
    if (config.ticks == 0) {
        throw std::invalid_argument("ticks must be greater than 0");
    }
}

std::string build_csv_path(std::uint32_t seed) {
    const std::filesystem::path output_dir = "output/data";
    std::filesystem::create_directories(output_dir);
    return (output_dir / ("evolution_stats_" + std::to_string(seed) + ".csv")).string();
}

void print_usage(const char* exe) {
    std::cout << "Us: " << exe << " [mida_mon] [aigua] [menjar] [herbivors] [depredadors] [ticks] [seed]\n";
    std::cout << "Exemple: " << exe << " 100 12 25 80 12 5000 42\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc > 8) {
            print_usage(argv[0]);
            return 2;
        }

        SimulationConfig config;
        if (argc >= 2) config.world_size = parse_double(argv[1]);
        if (argc >= 3) config.water_sources = parse_size(argv[2]);
        if (argc >= 4) config.food_sources = parse_size(argv[3]);
        if (argc >= 5) config.initial_herbivores = parse_size(argv[4]);
        if (argc >= 6) config.initial_predators = parse_size(argv[5]);
        if (argc >= 7) config.ticks = parse_u64(argv[6]);
        if (argc >= 8) config.seed = static_cast<std::uint32_t>(parse_u64(argv[7]));

        validate_config(config);

        Simulation simulation(config);
        simulation.run();

        const std::string filename = build_csv_path(config.seed);
        simulation.write_csv(filename);
        std::cout << "\nSimulacio finalitzada. Dades: " << filename << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
