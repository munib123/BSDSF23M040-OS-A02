CC = gcc
CFLAGS = -Wall -g

# Directories
BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src

# Target
TARGET = $(BIN_DIR)/ls

# Main targets
all: $(TARGET)

$(TARGET): $(SRC_DIR)/ls-v1.5.0.c | $(BIN_DIR) $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Create directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -f bin/ls obj/*

.PHONY: all clean