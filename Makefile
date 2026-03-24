TARGET = tmcli
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = build/obj
BIN_DIR = build/bin

FINAL_TARGET = $(BIN_DIR)/$(TARGET)

CC = gcc
STATE_FILE ?= state.dat
CFLAGS = -Wall -Wextra -g -I$(INC_DIR)
CFLAGS += -DSTATE_FILE='"$(STATE_FILE)"'
LDLIBS = -lm

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(FINAL_TARGET)

$(FINAL_TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS)  $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

run: $(FINAL_TARGET)
	./$(FINAL_TARGET) $(ARGS)

clean:
	rm -rf build
