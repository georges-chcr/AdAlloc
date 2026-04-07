CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
LDFLAGS  := -lsqlite3

SRCS     := src/main.cc src/knapsack.cc src/db_writer.cc
TARGET   := bin/adalloc

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS) | bin
	$(CXX) $(CXXFLAGS) $(SRCS) -o $@ $(LDFLAGS)

bin:
	mkdir -p bin

clean:
	rm -rf bin

run: all
	./bin/adalloc
