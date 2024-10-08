# Compiler
CC = gcc

# Include and Library paths
INCS = -IC:\MinGW_SDL\include\SDL2
LIBS = -LC:\MinGW_SDL\lib

# Compiler Flags
CFLAGS = -std=c99 -Wall

# Linker Flags
LFLAGS = -lmingw32 -lSDL2main -lSDL2

# Source files
SRC_C = ./common/common.c
SRC_C_TILES = ./c/tiles.c
SRC_C_LEVEL = ./c/level.c

# Object files
OBJ_C = ./common/common.o
OBJ_C_TILES = tiles.o
OBJ_C_LEVEL = level.o

# Executable names
EXE_TILES = tiles.exe
EXE_LEVEL = level.exe

# Targets
all: $(EXE_TILES) $(EXE_LEVEL)

# Rule to compile common.c
$(OBJ_C): $(SRC_C)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

# Rule to compile tiles.c
$(EXE_TILES): $(SRC_C_TILES) $(OBJ_C)
	$(CC) $(SRC_C_TILES) $(OBJ_C) $(INCS) $(LIBS) $(CFLAGS) $(LFLAGS) -o $@

# Rule to compile level.c
$(EXE_LEVEL): $(SRC_C_LEVEL) $(OBJ_C)
	$(CC) $(SRC_C_LEVEL) $(OBJ_C) $(INCS) $(LIBS) $(CFLAGS) $(LFLAGS) -o $@

# Clean up build files
clean:
	rm -f $(OBJ_C) $(EXE_TILES) $(EXE_LEVEL)
