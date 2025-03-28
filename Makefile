CXX=g++
LD_FLAGS=-O3 -flto -lm -lvterm -licuuc -lSDL2 -lSDL2_ttf

WIN_CXX=x86_64-w64-mingw32-g++
WIN_LD_FLAGS=-O3 -flto -lm -lmingw32 -lvterm -licuuc -lSDL2main -lSDL2 -lSDL2_ttf -mwindows -I/usr/x86_64-w64-mingw32/

CXX_FLAGS=-std=c++20 -fno-rtti -O3 -Iinclude/
SRCS=$(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJS=$(SRCS:src/%.cpp=src/%.o)
EXEC=rv64-emu

.PHONY: all help link linux linux-pack win win-pack test clean

help:
	@echo "Usage: make [linux|win]"

all: help

linux-pack: linux
linux-pack:
	tar -czf EmulatorRV64_linux.tar.gz $(EXEC) -C test/linux/ .

linux: link

win-pack: win
win-pack:
	zip -j EmulatorRV64_win.zip $(EXEC) winlib/*

win: CXX=$(WIN_CXX)
win: LD_FLAGS=$(WIN_LD_FLAGS)
win: link

link: $(OBJS) src/main.o
	@echo "LINK $(EXEC)"
	@$(CXX) $(OBJS) src/main.o -o $(EXEC) $(LD_FLAGS)

test: CXX_FLAGS+=-DEMU_DEBUG
test: $(OBJS) test/test.o
	@echo "LINK tmp_test"
	@$(CXX) $(OBJS) test/test.o -o tmp_test $(LD_FLAGS)
	@./tmp_test 2>/dev/null
	@rm tmp_test

%.o: %.cpp
	@echo "CXX $<"
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	@echo "CLEANING..."
	@rm -rf src/*.o
