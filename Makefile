CXX=g++
CXX_FLAGS=-std=c++20 -O3 -Iinclude/
LD_FLAGS=-lSDL2
SRCS=$(wildcard src/*.cpp)
OBJS=$(SRCS:src/%.cpp=src/%.o)
EXEC=rv64-emu

.PHONY: all clean

all: $(EXEC)
	./$(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LD_FLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	rm -rf src/*.o
