INC_DIR=inc
CC=gcc
# CFLAGS=-I$(INC_DIR) -Wall -M
CFLAGS=-Wall -g

SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
TARGET=main

SRCS=$(wildcard $(SRC_DIR)/*.c)
SRCS_NOTDIR=$(notdir $(SRCS))
OBJS=$(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS_NOTDIR))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(BIN_DIR)/$(TARGET) # -levent

all:
	@echo $(SRCS)
	@echo $(SRCS_NOTDIR)
	@echo $(OBJS)

.PHONY:clean

clean:
	rm $(OBJS) $(BIN_DIR)/$(TARGET)

