CXX=g++
CXX_FLAGS=
SRCS=$(wildcard src/*.cpp)
EXEC=rv64-emu

.PHONY: all link clean

all: link
	./$(EXEC)

link:
	$(CXX) $(CXX_FLAGS) src/main.cpp src/memory.cpp src/emulator.cpp -o $(EXEC)

clean:
	rm -rf src/*.o
