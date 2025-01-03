CXX=g++
CXX_FLAGS=-std=c++20 -g3 -O3 -Iinclude/
LD_FLAGS=-O3 -g3 -lm -licuuc -lvterm -lSDL2 -lSDL2_ttf
SRCS=$(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJS=$(SRCS:src/%.cpp=src/%.o)
EXEC=rv64-emu

.PHONY: all test clean

all: $(OBJS) src/main.o
	@echo "LINK $(EXEC)"
	@$(CXX) $(OBJS) src/main.o -o $(EXEC) $(LD_FLAGS)
	./$(EXEC)

test: CXX_FLAGS+=-DEMU_DEBUG
test: $(OBJS) test/test.o
	@echo "LINK tmp_test"
	@$(CXX) $(OBJS) test/test.o -o tmp_test $(LD_FLAGS)
	@./tmp_test 2>logs.txt
	@echo "Debug info dumped to logs.txt"
	@rm tmp_test

%.o: %.cpp
	@echo "CXX $<"
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	@echo "CLEANING..."
	@rm -rf src/*.o
