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

SRCS_CLI    = src/ui_cli.c
SRCS_GUI    = src/ui_gui.c
SRC_MAIN    = src/main.c

TARGET      = bin/serc-os
TARGET_CLI  = bin/serc-os-cli

OBJ_DIR     = obj

OBJS_COMMON = $(SRCS_COMMON:src/%.c=$(OBJ_DIR)/%.o)
OBJ_CLI     = $(OBJ_DIR)/ui_cli.o
OBJ_GUI     = $(OBJ_DIR)/ui_gui.o
OBJ_MAIN    = $(OBJ_DIR)/main.o

.PHONY: all cli clean dirs

all: dirs $(TARGET)

cli: CFLAGS  += -DSERC_CLI_ONLY
cli: LDFLAGS  =
cli: dirs $(TARGET_CLI)

$(TARGET): $(OBJS_COMMON) $(OBJ_CLI) $(OBJ_GUI) $(OBJ_MAIN)
	$(CC) $^ -o $@ $(LDFLAGS) $(GTK_LDFLAGS)

$(TARGET_CLI): $(OBJS_COMMON) $(OBJ_CLI) $(OBJ_DIR)/main_cli.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(OBJ_DIR)/ui_gui.o: src/ui_gui.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(OBJ_DIR)/main_cli.o: src/main.c
	$(CC) $(CFLAGS) -DSERC_CLI_ONLY -c $< -o $@

dirs:
	mkdir -p $(OBJ_DIR) bin logs

clean:
	rm -rf $(OBJ_DIR) bin
