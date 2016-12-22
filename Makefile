OS=$(shell uname)
CC=g++
C_FILES=main.cpp gameoflife.cpp sdl_window.cpp
H_FILES=gameoflife.h sdl_window.h
FLAGS=-Wall -std=c++11
EXE=gameoflife

ifeq ($(OS), Darwin)
	FLAGS+=-F/Library/Frameworks -framework SDL2 -framework OpenCL -framework OpenGL
else ifeq ($(OS), Linux)
	FLAGS+=-lSDL2 -lOpenCL -lGL
endif

all: $(EXE)

$(EXE): $(C_FILES) $(H_FILES)
	$(CC) $(FLAGS) $(C_FILES) -o $(EXE)

clean:
	rm -f $(EXE)
