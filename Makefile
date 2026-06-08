CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -fno-asynchronous-unwind-tables
LDFLAGS = -z relro

GTK_CFLAGS  = $(shell pkg-config --cflags gtk+-3.0 2>/dev/null)
GTK_LDFLAGS = $(shell pkg-config --libs   gtk+-3.0 2>/dev/null)

SRCS_COMMON = src/logger.c       \
              src/process.c      \
              src/scheduler.c    \
              src/memory.c       \
              src/ipc.c          \
              src/deadlock.c     \
              src/filemanager.c

TARGET     = bin/serc-os
TARGET_CLI = bin/serc-os-cli

OBJ_DIR     = obj/full
OBJ_DIR_CLI = obj/cli

OBJS_FULL = $(SRCS_COMMON:src/%.c=$(OBJ_DIR)/%.o) \
            $(OBJ_DIR)/ui_cli.o                    \
            $(OBJ_DIR)/ui_gui.o                    \
            $(OBJ_DIR)/main.o

OBJS_CLI  = $(SRCS_COMMON:src/%.c=$(OBJ_DIR_CLI)/%.o) \
            $(OBJ_DIR_CLI)/ui_cli.o                    \
            $(OBJ_DIR_CLI)/main_cli.o

.PHONY: all cli clean dirs

all: dirs $(TARGET)
cli: dirs $(TARGET_CLI)

$(TARGET): $(OBJS_FULL)
	$(CC) $^ -o $@ $(LDFLAGS) $(GTK_LDFLAGS)

$(TARGET_CLI): $(OBJS_CLI)
	$(CC) $^ -o $@

# Full build — GTK enabled, no CLI_ONLY flag
$(OBJ_DIR)/ui_gui.o: src/ui_gui.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# CLI build — CLI_ONLY flag, no GTK
$(OBJ_DIR_CLI)/main_cli.o: src/main.c
	$(CC) $(CFLAGS) -DSERC_CLI_ONLY -c $< -o $@

$(OBJ_DIR_CLI)/%.o: src/%.c
	$(CC) $(CFLAGS) -DSERC_CLI_ONLY -c $< -o $@

dirs:
	mkdir -p $(OBJ_DIR) $(OBJ_DIR_CLI) bin logs

clean:
	rm -rf obj bin