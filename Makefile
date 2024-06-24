TARGET = main

DEBUG ?= 1

# Direcotries
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
LIB_DIR = lib

# Compiler
CC = gcc 
CFLAGS = -xc -Wall -I$(INC_DIR) -I$(LIB_DIR)
LDFLAGS = -lmenu -lncurses

# Files 
SRC = $(wildcard $(SRC_DIR)/*.c) 
LIB = $(wildcard $(LIB_DIR)/**/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC)) \
      $(patsubst $(LIB_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB))

# Rules
ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

all: directories $(BIN_DIR)/$(TARGET)

directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

$(BIN_DIR)/$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean directories
