CXX=g++
CXX_FLAGS=-Iinclude/
SRCS=$(wildcard src/*.cpp)
EXEC=rv64-emu

.PHONY: all link clean

all: link
	./$(EXEC)

link:
	$(CXX) $(CXX_FLAGS) $(SRCS) -o $(EXEC)

clean:
	rm -rf src/*.o
