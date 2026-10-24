CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
TARGET   := order_os_visualizer
SRC      := $(wildcard src/*.cpp)

all: $(TARGET)

$(TARGET): $(SRC) $(wildcard src/*.hpp)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

test: $(TARGET)
	./tests/run_tests.sh ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all test clean
