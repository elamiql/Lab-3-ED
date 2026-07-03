#include <iostream>
#include <fstream>
#include <vector>


#include "csv_reader.hpp"
#include "benchmark.hpp" 
#include "chaining_table.hpp"
#include "hash_functions.hpp"

int main() {

    // 1. Cargar datos
    std::vector<Tweet> dataset = CSVReader::read_tweets("tweets.csv");
    if (dataset.empty()) return 1;

    // 2. Preparar archivo de salida
    std::ofstream output_file("benchmark_results.csv");
    output_file << "run_id;structure;collision;key_type;n_tweets;time_ms;size_bytes\n";

    int run_id = 1;

    //Chaining + Separate + Int
    run_chaining_benchmark<int64_t, UserIdHash1>(
        output_file, run_id, dataset, "separate", "int", 
        [](const Tweet& t) { return t.user_id; }
    );

    // Chaining + Separate + String
    run_chaining_benchmark<std::string, ScreenNameHash1>(
        output_file, run_id, dataset, "separate", "string", 
        [](const Tweet& t) { return t.user_screen_name; }
    );


    // Open + Linear + Int
    run_open_addressing_benchmark<int64_t, UserIdHash1, UserIdHash2>(
        output_file, run_id, dataset, ProbingStrategy::LINEAR, "linear", "int",
        [](const Tweet& t) { return t.user_id; }
    );
    // Open + Linear + String
    run_open_addressing_benchmark<std::string, ScreenNameHash1, ScreenNameHash2>(
        output_file, run_id, dataset, ProbingStrategy::LINEAR, "linear", "string",
        [](const Tweet& t) { return t.user_screen_name; }
    );

    
    // Open + Quadratic + Int
    run_open_addressing_benchmark<int64_t, UserIdHash1, UserIdHash2>(
        output_file, run_id, dataset, ProbingStrategy::QUADRATIC, "quadratic", "int",
        [](const Tweet& t) { return t.user_id; }
    );
    //Open + Quadratic + String
    run_open_addressing_benchmark<std::string, ScreenNameHash1, ScreenNameHash2>(
        output_file, run_id, dataset, ProbingStrategy::QUADRATIC, "quadratic", "string",
        [](const Tweet& t) { return t.user_screen_name; }
    );

    
    // Open + Double Hash + Int
    run_open_addressing_benchmark<int64_t, UserIdHash1, UserIdHash2>(
        output_file, run_id, dataset, ProbingStrategy::DOUBLE_HASHING, "double_hashing", "int",
        [](const Tweet& t) { return t.user_id; }
    );
    // Open + Double Hash + String
    run_open_addressing_benchmark<std::string, ScreenNameHash1, ScreenNameHash2>(
        output_file, run_id, dataset, ProbingStrategy::DOUBLE_HASHING, "double_hashing", "string",
        [](const Tweet& t) { return t.user_screen_name; }
    );

    output_file.close();
    std::cout << "[+] Benchmarks finalizados con exito. " << (run_id - 1) << " corridas registradas." << std::endl;
    return 0;
}