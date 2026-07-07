#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "chaining_table.hpp"
#include "hash_functions.hpp"
#include "open_addressing.hpp"

struct Tweet {
    int64_t user_id;
    std::string screen_name;
};

// Lee el CSV de tweets completo, extrayendo solo las columnas user_id
// (indice 5) y user_screen_name (indice 7). Respeta comillas y comas/saltos
// de linea dentro de campos citados (ej. full_text puede traer de todo eso).
std::vector<Tweet> load_tweets(const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("No se pudo abrir el archivo: " + path);
    }

    std::vector<Tweet> tweets;
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    bool first_row = true; // la primera fila es el header, se descarta
    char c;

    auto finish_row = [&]() {
        fields.push_back(field);
        field.clear();

        if (!first_row && fields.size() > 7) {
            try {
                Tweet t;
                t.user_id = std::stoll(fields[5]);
                t.screen_name = fields[7];
                tweets.push_back(std::move(t));
            } catch (...) {
                // fila con user_id no numerico o corrupta: se ignora
            }
        }
        first_row = false;
        fields.clear();
    };

    while (file.get(c)) {
        if (in_quotes) {
            if (c == '"') {
                if (file.peek() == '"') {
                    field += '"';
                    file.get(c); // consume la comilla duplicada (escape)
                } else {
                    in_quotes = false;
                }
            } else {
                field += c;
            }
        } else {
            if (c == '"') {
                in_quotes = true;
            } else if (c == ',') {
                fields.push_back(field);
                field.clear();
            } else if (c == '\r') {
                continue;
            } else if (c == '\n') {
                finish_row();
            } else {
                field += c;
            }
        }
    }
    if (!field.empty() || !fields.empty()) {
        finish_row();
    }

    return tweets;
}

// Corre las 5 estructuras (chaining, linear, quadratic, double_hashing,
// unordered_map) para una clave especifica (user_id o screen_name),
// repitiendo num_repetitions veces y escribiendo cada checkpoint al csv.
template <typename Key, typename Hash1, typename Hash2>
void run_experiments_for_key(const std::vector<Tweet> &tweets,
                             std::function<Key(const Tweet &)> key_selector,
                             const std::string &key_type_label,
                             int num_repetitions,
                             const std::vector<int> &checkpoints,
                             std::ofstream &out, int &run_id) {

    for (int rep = 1; rep <= num_repetitions; rep++) {
        std::cout << "  [" << key_type_label << "] repeticion " << rep << "/"
                  << num_repetitions << "\n";

        // ---- Chaining (hashing abierto) ----
        {
            ChainingHashTable<Key, Hash1> table(1024);
            auto start = std::chrono::high_resolution_clock::now();
            size_t ckpt_idx = 0;
            for (size_t i = 0; i < tweets.size(); i++) {
                table.increment(key_selector(tweets[i]));
                if (ckpt_idx < checkpoints.size() &&
                    static_cast<int>(i + 1) == checkpoints[ckpt_idx]) {
                    auto now = std::chrono::high_resolution_clock::now();
                    double ms =
                        std::chrono::duration<double, std::milli>(now - start)
                            .count();
                    out << (run_id++) << ";chaining;separate;" << key_type_label
                        << ";" << checkpoints[ckpt_idx] << ";" << ms << ";"
                        << table.memory_bytes() << "\n";
                    ckpt_idx++;
                }
            }
        }

        // ---- Open addressing: linear / quadratic / double hashing ----
        struct Strategy {
            ProbingStrategy strategy;
            const char *name;
        };
        Strategy strategies[] = {
            {ProbingStrategy::LINEAR, "linear"},
            {ProbingStrategy::QUADRATIC, "quadratic"},
            {ProbingStrategy::DOUBLE_HASHING, "double_hashing"},
        };

        for (const auto &s : strategies) {
            OpenAddressingHashTable<Key, Hash1, Hash2> table(1024, s.strategy);
            auto start = std::chrono::high_resolution_clock::now();
            size_t ckpt_idx = 0;
            for (size_t i = 0; i < tweets.size(); i++) {
                table.increment(key_selector(tweets[i]));
                if (ckpt_idx < checkpoints.size() &&
                    static_cast<int>(i + 1) == checkpoints[ckpt_idx]) {
                    auto now = std::chrono::high_resolution_clock::now();
                    double ms =
                        std::chrono::duration<double, std::milli>(now - start)
                            .count();
                    out << (run_id++) << ";open_addressing;" << s.name << ";"
                        << key_type_label << ";" << checkpoints[ckpt_idx] << ";"
                        << ms << ";" << table.memory_bytes() << "\n";
                    ckpt_idx++;
                }
            }
        }

        // ---- unordered_map de la STL ----
        {
            std::unordered_map<Key, int> table;
            table.reserve(1024);
            auto start = std::chrono::high_resolution_clock::now();
            size_t ckpt_idx = 0;
            for (size_t i = 0; i < tweets.size(); i++) {
                table[key_selector(tweets[i])]++;
                if (ckpt_idx < checkpoints.size() &&
                    static_cast<int>(i + 1) == checkpoints[ckpt_idx]) {
                    auto now = std::chrono::high_resolution_clock::now();
                    double ms =
                        std::chrono::duration<double, std::milli>(now - start)
                            .count();
                    size_t bucket_array = table.bucket_count() * sizeof(void *);
                    size_t node_overhead =
                        sizeof(Key) + sizeof(int) + 2 * sizeof(void *);
                    size_t mem = bucket_array + table.size() * node_overhead;
                    out << (run_id++) << ";unordered_map;none;"
                        << key_type_label << ";" << checkpoints[ckpt_idx] << ";"
                        << ms << ";" << mem << "\n";
                    ckpt_idx++;
                }
            }
        }
    }
}

int main() {
    // AJUSTAR si el dataset tiene otro nombre o ruta
    const std::string data_path = "data/auspol2019.csv";
    const std::string output_path = "benchmark_results.csv";
    const int NUM_REPETITIONS = 20;

    std::cout << "Cargando tweets desde " << data_path << "...\n";
    std::vector<Tweet> tweets = load_tweets(data_path);
    std::cout << "Tweets cargados: " << tweets.size() << "\n";

    if (tweets.empty()) {
        std::cerr << "No se cargaron tweets. Revisa la ruta del dataset.\n";
        return 1;
    }

    std::vector<int> checkpoints;
    for (int n = 10000; n <= 180000; n += 10000) {
        checkpoints.push_back(n);
    }
    // Si el dataset tiene menos de 180000 filas, se ajusta el ultimo
    // checkpoint al total real disponible.
    if (static_cast<int>(tweets.size()) < checkpoints.back()) {
        while (!checkpoints.empty() &&
               checkpoints.back() > static_cast<int>(tweets.size())) {
            checkpoints.pop_back();
        }
        checkpoints.push_back(static_cast<int>(tweets.size()));
    }

    std::filesystem::create_directories("results");
    std::ofstream out(output_path);
    out << "run_id;structure;collision;key_type;n_tweets;time_ms;size_bytes\n";

    int run_id = 1;

    std::cout << "Corriendo experimentos con clave user_id...\n";
    run_experiments_for_key<int64_t, UserIdHash1, UserIdHash2>(
        tweets, [](const Tweet &t) { return t.user_id; }, "int",
        NUM_REPETITIONS, checkpoints, out, run_id);

    std::cout << "Corriendo experimentos con clave user_screen_name...\n";
    run_experiments_for_key<std::string, ScreenNameHash1, ScreenNameHash2>(
        tweets, [](const Tweet &t) { return t.screen_name; }, "string",
        NUM_REPETITIONS, checkpoints, out, run_id);

    out.close();
    std::cout << "Listo. Resultados en " << output_path << "\n";
    return 0;
}