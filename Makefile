CXX=g++
CXX_FLAGS=
SRCS=$(wildcard src/*.cpp)
EXEC=rv32-emu

.PHONY: all link clean

all: link
	./$(EXEC)

link:
	$(CXX) $(CXX_FLAGS) $(SRCS) -o $(EXEC)

clean:
	rm -rf src/*.o
