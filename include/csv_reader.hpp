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

        std::vector<std::string> fields;
        std::string field;
        bool in_quotes = false;
        bool header_skipped = false;

        auto flush_record = [&]() {
            if (fields.empty() && field.empty()) {
                return;
            }

            fields.push_back(field);
            field.clear();

            if (!header_skipped) {
                header_skipped = true;
                fields.clear();
                return;
            }

            if (fields.size() >= 8) {
                try {
                    Tweet tweet;
                    tweet.user_id = std::stoll(fields[5]);
                    tweet.user_screen_name = fields[7];
                    tweets.push_back(tweet);
                } catch (const std::exception&) {
                    // Saltar registros malformados sin detener el benchmark.
                }
            }

            fields.clear();
        };

        char ch;
        while (file.get(ch)) {
            if (ch == '\r') {
                continue;
            }

            if (ch == '"') {
                if (in_quotes && file.peek() == '"') {
                    field.push_back('"');
                    file.get(ch);
                } else {
                    in_quotes = !in_quotes;
                }
            } else if (ch == ',' && !in_quotes) {
                fields.push_back(field);
                field.clear();
            } else if (ch == '\n' && !in_quotes) {
                flush_record();
            } else {
                field.push_back(ch);
            }
        }

        flush_record();

        file.close();
        return tweets;
    }
};