#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include "csv_reader.hpp"
#include "chaining_table.hpp"
#include "open_addressing.hpp"

// Función para automatizar los combos de ChainingHashTable
template <typename KeyType, typename HashClass>
void run_chaining_benchmark(
    std::ofstream& output_file, 
    int& run_id, 
    const std::vector<Tweet>& dataset, 
    const std::vector<size_t>& checkpoints,
    const std::string& collision_strategy, 
    const std::string& key_type_name, 
    auto key_extractor 
) {
    const int REPETICIONES = 20;

    for (int r = 0; r < REPETICIONES; ++r) {
        // Inicializamos la tabla en cada repetición para limpiar memoria
        ChainingHashTable<KeyType, HashClass> table(1024);

        auto start = std::chrono::high_resolution_clock::now();

        size_t checkpoint_index = 0;
        for (size_t tweet_index = 0; tweet_index < dataset.size() && checkpoint_index < checkpoints.size(); ++tweet_index) {
            const auto& tweet = dataset[tweet_index];
            table.increment(key_extractor(tweet));

            size_t processed = tweet_index + 1;
            while (checkpoint_index < checkpoints.size() && processed >= checkpoints[checkpoint_index]) {
                auto now = std::chrono::high_resolution_clock::now();
                double time_ms = std::chrono::duration<double, std::milli>(now - start).count();
                size_t size_bytes = table.memory_bytes();

                output_file << run_id++ << ";chaining;" << collision_strategy << ";"
                            << key_type_name << ";" << checkpoints[checkpoint_index] << ";"
                            << time_ms << ";" << size_bytes << "\n";
                ++checkpoint_index;
            }
        }
    }
}

// Función para automatizar los combos de OpenAddressingHashTable
template <typename KeyType, typename Hash1, typename Hash2>
void run_open_addressing_benchmark(
    std::ofstream& output_file, 
    int& run_id, 
    const std::vector<Tweet>& dataset, 
    const std::vector<size_t>& checkpoints,
    ProbingStrategy strategy,
    const std::string& collision_strategy_name, 
    const std::string& key_type_name, 
    auto key_extractor
) {
    const int REPETICIONES = 20;

    for (int r = 0; r < REPETICIONES; ++r) {
        OpenAddressingHashTable<KeyType, Hash1, Hash2> table(1024, strategy);

        auto start = std::chrono::high_resolution_clock::now();

        size_t checkpoint_index = 0;
        for (size_t tweet_index = 0; tweet_index < dataset.size() && checkpoint_index < checkpoints.size(); ++tweet_index) {
            const auto& tweet = dataset[tweet_index];
            table.increment(key_extractor(tweet));

            size_t processed = tweet_index + 1;
            while (checkpoint_index < checkpoints.size() && processed >= checkpoints[checkpoint_index]) {
                auto now = std::chrono::high_resolution_clock::now();
                double time_ms = std::chrono::duration<double, std::milli>(now - start).count();
                size_t size_bytes = table.memory_bytes();

                output_file << run_id++ << ";open_addressing;" << collision_strategy_name << ";"
                            << key_type_name << ";" << checkpoints[checkpoint_index] << ";"
                            << time_ms << ";" << size_bytes << "\n";
                ++checkpoint_index;
            }
        }
    }
}