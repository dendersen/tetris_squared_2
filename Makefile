CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -O0 -I./src
LDFLAGS :=

SRC_DIR := src
BIN_DIR := bin
OUT := out

SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(OUT)

-include $(DEPS)
