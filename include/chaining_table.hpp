#pragma once
#include <vector>
#include <list>
#include <utility>
#include <cstddef>

// Tabla hash con hashing abierto (encadenamiento separado).
// Costo insercion/busqueda promedio: O(1 + factor_de_carga).
// Costo peor caso: O(n) si todas las claves colisionan en el mismo bucket.
//
// Key:  tipo de la clave (int64_t o std::string)
// Hash: functor que implementa size_t operator()(const Key&) const
template <typename Key, typename Hash> class ChainingHashTable {
  public:
    explicit ChainingHashTable(size_t initial_buckets = 1024)
        : buckets_(initial_buckets == 0 ? 1 : initial_buckets), num_keys_(0) {}

    // Implementa: if (k in H) H[k]++; else H[k] = 1;
    void increment(const Key &key) {
        size_t idx = hasher_(key) % buckets_.size();
        for (auto &entry : buckets_[idx]) {
            if (entry.first == key) {
                entry.second += 1;
                return;
            }
        }
        buckets_[idx].emplace_front(key, 1);
        num_keys_++;
        if (load_factor() > MAX_LOAD_FACTOR) {
            rehash();
        }
    }

    size_t size() const { return num_keys_; }
    size_t bucket_count() const { return buckets_.size(); }
    double load_factor() const {
        return static_cast<double>(num_keys_) /
               static_cast<double>(buckets_.size());
    }

    // Estimacion de memoria en bytes: arreglo de punteros a bucket +
    // overhead de nodo de lista enlazada por cada clave almacenada.
    size_t memory_bytes() const {
        size_t bucket_array = buckets_.size() * sizeof(void *);
        size_t node_overhead = sizeof(Key) + sizeof(int) + 2 * sizeof(void *);
        return bucket_array + num_keys_ * node_overhead;
    }

  private:
    static constexpr double MAX_LOAD_FACTOR = 1.0;

    std::vector<std::list<std::pair<Key, int>>> buckets_;
    size_t num_keys_;
    Hash hasher_;

    void rehash() {
        size_t new_bucket_count = buckets_.size() * 2;
        std::vector<std::list<std::pair<Key, int>>> new_buckets(
            new_bucket_count);
        for (auto &bucket : buckets_) {
            for (auto &entry : bucket) {
                size_t idx = hasher_(entry.first) % new_bucket_count;
                new_buckets[idx].push_back(std::move(entry));
            }
        }
        buckets_ = std::move(new_buckets);
    }
};