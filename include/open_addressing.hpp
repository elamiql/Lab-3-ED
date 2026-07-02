#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>

enum class ProbingStrategy { LINEAR, QUADRATIC, DOUBLE_HASHING };

// Tabla hash con hashing cerrado (open addressing).
// Costo insercion/busqueda promedio: O(1 / (1 - factor_de_carga)).
// Costo peor caso: O(n).
//
// Key:   tipo de la clave (int64_t o std::string)
// Hash1: hash primaria, size_t operator()(const Key&) const
// Hash2: hash secundaria, solo se usa si strategy == DOUBLE_HASHING
//        (debe garantizar nunca retornar 0)
//
// Nota sobre quadratic probing: se usa la forma simple (h1 + i^2) % cap.
// Esto no garantiza visitar el 100% de los slots en tablas no primas al
// cuadrado, pero es la variante estandar vista en el curso; se compensa
// haciendo rehash antes de que la tabla se llene (MAX_LOAD_FACTOR = 0.7).
template <typename Key, typename Hash1, typename Hash2>
class OpenAddressingHashTable {
  public:
    OpenAddressingHashTable(size_t initial_capacity, ProbingStrategy strategy)
        : capacity_(next_prime(initial_capacity == 0 ? 2 : initial_capacity)),
          num_keys_(0), strategy_(strategy) {
        table_.resize(capacity_);
    }

    // Implementa: if (k in H) H[k]++; else H[k] = 1;
    void increment(const Key &key) {
        size_t idx = find_slot(key, table_, capacity_);
        if (table_[idx].state == SlotState::OCCUPIED) {
            table_[idx].count += 1;
        } else {
            table_[idx].key = key;
            table_[idx].count = 1;
            table_[idx].state = SlotState::OCCUPIED;
            num_keys_++;
            if (load_factor() > MAX_LOAD_FACTOR) {
                rehash();
            }
        }
    }

    size_t size() const { return num_keys_; }
    size_t capacity() const { return capacity_; }
    double load_factor() const {
        return static_cast<double>(num_keys_) / static_cast<double>(capacity_);
    }

    // Estimacion de memoria en bytes: capacidad * tamano de cada slot.
    size_t memory_bytes() const { return capacity_ * sizeof(Slot); }

  private:
    enum class SlotState { EMPTY, OCCUPIED };

    struct Slot {
        Key key{};
        int count = 0;
        SlotState state = SlotState::EMPTY;
    };

    static constexpr double MAX_LOAD_FACTOR = 0.7;

    std::vector<Slot> table_;
    size_t capacity_;
    size_t num_keys_;
    ProbingStrategy strategy_;
    Hash1 hash1_;
    Hash2 hash2_;

    // Busca el slot donde deberia estar 'key': si ya existe, retorna su
    // posicion; si no existe, retorna la primera posicion libre.
    size_t find_slot(const Key &key, std::vector<Slot> &table,
                     size_t cap) const {
        size_t h1 = hash1_(key) % cap;
        for (size_t attempt = 0; attempt < cap; attempt++) {
            size_t idx = probe(h1, key, attempt, cap);
            if (table[idx].state != SlotState::OCCUPIED ||
                table[idx].key == key) {
                return idx;
            }
        }
        throw std::runtime_error(
            "Tabla llena: no deberia pasar si el rehash funciona bien");
    }

    size_t probe(size_t h1, const Key &key, size_t attempt, size_t cap) const {
        switch (strategy_) {
        case ProbingStrategy::LINEAR:
            return (h1 + attempt) % cap;
        case ProbingStrategy::QUADRATIC:
            return (h1 + attempt * attempt) % cap;
        case ProbingStrategy::DOUBLE_HASHING: {
            size_t h2 = hash2_(key);
            return (h1 + attempt * h2) % cap;
        }
        }
        return h1;
    }

    void rehash() {
        size_t new_capacity = next_prime(capacity_ * 2);
        std::vector<Slot> new_table(new_capacity);
        for (auto &slot : table_) {
            if (slot.state == SlotState::OCCUPIED) {
                size_t idx = find_slot(slot.key, new_table, new_capacity);
                new_table[idx] = slot;
            }
        }
        table_ = std::move(new_table);
        capacity_ = new_capacity;
    }

    static bool is_prime(size_t n) {
        if (n < 2)
            return false;
        for (size_t i = 2; i * i <= n; i++) {
            if (n % i == 0)
                return false;
        }
        return true;
    }

    static size_t next_prime(size_t n) {
        if (n <= 2)
            return 2;
        size_t candidate = (n % 2 == 0) ? n + 1 : n;
        while (!is_prime(candidate))
            candidate += 2;
        return candidate;
    }
};