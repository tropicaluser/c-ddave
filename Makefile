# Compiler
CC = gcc

# Include and Library paths
INCS = -IC:\MinGW_SDL\include\SDL2
LIBS = -LC:\MinGW_SDL\lib

# Compiler and Linker Flags
CFLAGS = -std=c99 -Wall
LFLAGS = -lmingw32 -lSDL2main -lSDL2

# Source files
SRC_C = ./common/common.c
SRC_C_GAME = ./common/game.c
SRC_C_ALL = $(SRC_C) $(SRC_C_GAME)

# C Executables and source files mapping
EXE_FILES = tiles level imdave
SRC_FILES_tiles = ./c/tiles.c
SRC_FILES_level = ./c/level.c
SRC_FILES_imdave = ./c/imdave.c

# Object files
OBJ_C = ./common/common.o
OBJ_C_GAME = ./common/game.o

# Targets
all: clean_exe $(EXE_FILES)

# Rule to clean up object files and executables
clean_exe:
	rm -f $(EXE_FILES:=.exe) $(OBJ_C) $(OBJ_C_GAME)

# Generic rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

# Rule to build each executable
$(EXE_FILES): %: ./c/%.c $(OBJ_C) $(OBJ_C_GAME)
	$(CC) $< $(OBJ_C) $(OBJ_C_GAME) $(INCS) $(LIBS) $(CFLAGS) $(LFLAGS) -o $@.exe

# Clean up all build files
clean:
	rm -f $(OBJ_C) $(OBJ_C_GAME) $(EXE_FILES:=.exe)
