CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Iinclude

# src/main.cpp: el harness de experimentos (Parte 2), genera results.csv
MAIN_SRC = src/main.cpp

ifeq ($(OS),Windows_NT)
    MAIN_BIN = bin/run_experiments.exe
    MKDIR_CMD = if not exist bin mkdir bin
    CLEAN_CMD = if exist bin rmdir /s /q bin
    RUN_CMD = bin\run_experiments.exe
else
    MAIN_BIN = bin/run_experiments
    MKDIR_CMD = mkdir -p bin
    CLEAN_CMD = rm -rf bin
    RUN_CMD = ./$(MAIN_BIN)
endif

all: $(MAIN_BIN)

$(MAIN_BIN): $(MAIN_SRC) include/*.hpp
	$(MKDIR_CMD)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(MAIN_BIN)

run: all
	$(RUN_CMD)

clean:
	$(CLEAN_CMD)

.PHONY: all run clean