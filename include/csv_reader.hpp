#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>

// Estructura limpia que almacena los dos datos clave del tweet necesarios para los hashes
struct Tweet {
    int64_t user_id;
    std::string user_screen_name;
};

class CSVReader {
public:
    // Lee el archivo CSV completo y lo carga en memoria 
    static inline std::vector<Tweet> read_tweets(const std::string& filepath) {
        std::vector<Tweet> tweets;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            std::cerr << "[-] Error: No se pudo abrir el archivo en la ruta: " << filepath << std::endl;
            return tweets;
        }

        std::string line;
    
        if (!std::getline(file, line)) {
            return tweets;
        }

        // Leer registro por registro
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string token_id;
            std::string token_name;

            // Suponiendo formato estándar delimitado por punto y coma 
            if (std::getline(ss, token_id, ';') && std::getline(ss, token_name, ';')) {
                try {
                    Tweet tweet;
                    tweet.user_id = std::stoll(token_id); 
                    tweet.user_screen_name = token_name;
                    tweets.push_back(tweet);
                } catch (const std::exception& e) {
                    continue; 
                }
            }
        }

        file.close();
        return tweets;
    }
};