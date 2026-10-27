CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -pthread

SRC := $(wildcard src/task*.cpp)
BIN := $(patsubst src/%.cpp,bin/%,$(SRC))

all: $(BIN)

bin/%: src/%.cpp | bin
	$(CXX) $(CXXFLAGS) $< -o $@

bin:
	mkdir -p bin

run: all
	./run_all.sh

clean:
	rm -rf bin actual_run.txt

.PHONY: all run clean
