# Compiler and Flags
CC = gcc
LDFLAGS = -lreadline
CFLAGS = -Iinclude -Wall
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
BIN = bin/myshell

# Default target
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

# Linking step

# Compilation step for each .c → .o
obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf obj/*.o $(BIN)

