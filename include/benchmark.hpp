#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include "csv_reader.hpp"
#include "open_addressing.hpp"

// Función para automatizar los combos de ChainingHashTable
template <typename KeyType, typename HashClass>
void run_chaining_benchmark(
    std::ofstream& output_file, 
    int& run_id, 
    const std::vector<Tweet>& dataset, 
    const std::string& collision_strategy, 
    const std::string& key_type_name, 
    auto key_extractor 
) {
    const int REPETICIONES = 20;
    size_t n_tweets = dataset.size();

    for (int r = 0; r < REPETICIONES; ++r) {
        // Inicializamos la tabla en cada repetición para limpiar memoria
        ChainingHashTable<KeyType, HashClass> table(1024);

        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& tweet : dataset) {
            table.increment(key_extractor(tweet));
        }
        auto end = std::chrono::high_resolution_clock::now();

        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        size_t size_bytes = table.memory_bytes();

        // Escribir directo al archivo con el formato exacto requerido
        output_file << run_id++ << ";chaining;" << collision_strategy << ";" 
                    << key_type_name << ";" << n_tweets << ";" << time_ms << ";" << size_bytes << "\n";
    }
}

// Función para automatizar los combos de OpenAddressingHashTable
template <typename KeyType, typename Hash1, typename Hash2>
void run_open_addressing_benchmark(
    std::ofstream& output_file, 
    int& run_id, 
    const std::vector<Tweet>& dataset, 
    ProbingStrategy strategy,
    const std::string& collision_strategy_name, 
    const std::string& key_type_name, 
    auto key_extractor
) {
    const int REPETICIONES = 20;
    size_t n_tweets = dataset.size();

    for (int r = 0; r < REPETICIONES; ++r) {
        OpenAddressingHashTable<KeyType, Hash1, Hash2> table(1024, strategy);

        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& tweet : dataset) {
            table.increment(key_extractor(tweet));
        }
        auto end = std::chrono::high_resolution_clock::now();

        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        size_t size_bytes = table.memory_bytes();

        output_file << run_id++ << ";open_addressing;" << collision_strategy_name << ";" 
                    << key_type_name << ";" << n_tweets << ";" << time_ms << ";" << size_bytes << "\n";
    }
}