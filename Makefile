CC = gcc
INCS = -IC:\MinGW_SDL\include\SDL2
LIBS = -LC:\MinGW_SDL\lib
CFLAGS = -std=c99 -Wall
LFLAGS = -lmingw32 -lSDL2main -lSDL2

SRC_C = ./common/common.c
SRC_C_MAIN = ./c/tiles.c
OBJ_C = ./common/common.o
OBJ_C_MAIN = TILES.exe

all: $(OBJ_C_MAIN)

$(OBJ_C): $(SRC_C)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(OBJ_C_MAIN): $(SRC_C_MAIN) $(OBJ_C)
	$(CC) $(SRC_C_MAIN) $(OBJ_C) $(INCS) $(LIBS) $(CFLAGS) $(LFLAGS) -o $@

clean:
	rm -f $(OBJ_C) $(OBJ_C_MAIN)
