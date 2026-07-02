#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

// Funciones hash para user_id (int64_t)

// Hash primaria: hashing multiplicativo (metodo de Knuth).
// Costo: O(1).
struct UserIdHash1 {
    size_t operator()(int64_t key) const {
        uint64_t x = static_cast<uint64_t>(key);
        return static_cast<size_t>(x * 2654435761ULL);
    }
};

// Hash secundaria (solo para double hashing). Debe cumplir dos cosas:
// nunca retornar 0 y ser "independiente" de la hash primaria.
// Costo: O(1).
struct UserIdHash2 {
    size_t operator()(int64_t key) const {
        uint64_t x = static_cast<uint64_t>(key);
        size_t h = static_cast<size_t>((x ^ (x >> 33)) * 0xff51afd7ed558ccdULL);
        return (h % 97) + 1; // rango [1, 97], nunca 0
    }
};

// Funciones hash para user_screen_name (std::string)

// Hash primaria: FNV-1a (64 bits). Costo: O(longitud del string).
struct ScreenNameHash1 {
    size_t operator()(const std::string &key) const {
        size_t hash = 14695981039346656037ULL; // FNV offset basis
        for (unsigned char c : key) {
            hash ^= c;
            hash *= 1099511628211ULL; // FNV prime
        }
        return hash;
    }
};

// Hash secundaria (solo para double hashing): variante djb2.
// Costo: O(longitud del string).
struct ScreenNameHash2 {
    size_t operator()(const std::string &key) const {
        size_t hash = 5381;
        for (unsigned char c : key) {
            hash = ((hash << 5) + hash) + c; // hash*33 + c
        }
        return (hash % 97) + 1; // rango [1, 97], nunca 0
    }
};