CC = "C:\Program Files\LLVM\bin\clang.exe"
CFLAGS = -Wall -g

SRCS = main.c logger.c ui.c process.c scheduler.c
OBJS = $(SRCS:.c=.o)
EXEC = semOS.exe

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o $(EXEC)
