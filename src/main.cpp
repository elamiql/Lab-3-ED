#include <iostream>
#include <string>
#include "chaining_table.hpp"
#include "open_addressing.hpp"
#include "hash_functions.hpp"

int main() {
    // --- Chaining con user_id ---
    ChainingHashTable<int64_t, UserIdHash1> chain_table(16);
    chain_table.increment(92484856);
    chain_table.increment(92484856);
    chain_table.increment(775647396);
    std::cout << "[Chaining/user_id] usuarios distintos: " << chain_table.size()
              << " | memoria (B): " << chain_table.memory_bytes() << "\n";

    // --- Linear probing con screen_name ---
    OpenAddressingHashTable<std::string, ScreenNameHash1, ScreenNameHash2>
        linear_table(16, ProbingStrategy::LINEAR);
    linear_table.increment("jocksjig");
    linear_table.increment("jocksjig");
    linear_table.increment("nparmar1957");
    std::cout << "[Linear/screen_name] usuarios distintos: "
              << linear_table.size()
              << " | memoria (B): " << linear_table.memory_bytes() << "\n";

    // --- Quadratic probing con user_id ---
    OpenAddressingHashTable<int64_t, UserIdHash1, UserIdHash2> quad_table(
        16, ProbingStrategy::QUADRATIC);
    quad_table.increment(908166034045026305LL);
    quad_table.increment(908166034045026305LL);
    std::cout << "[Quadratic/user_id] usuarios distintos: " << quad_table.size()
              << "\n";

    // --- Double hashing con user_id ---
    OpenAddressingHashTable<int64_t, UserIdHash1, UserIdHash2> double_table(
        16, ProbingStrategy::DOUBLE_HASHING);
    double_table.increment(111);
    double_table.increment(111);
    double_table.increment(222);
    std::cout << "[Double hashing/user_id] usuarios distintos: "
              << double_table.size() << "\n";

    // --- Prueba de rehash: insertar hartas claves distintas ---
    ChainingHashTable<int64_t, UserIdHash1> stress_table(4);
    for (int64_t i = 0; i < 10000; i++) {
        stress_table.increment(i);
    }
    std::cout << "[Stress test] claves: " << stress_table.size()
              << " | buckets finales: " << stress_table.bucket_count()
              << " | load factor: " << stress_table.load_factor() << "\n";

    std::cout << "Todo OK.\n";
    return 0;
}