CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lncurses
TARGET = snake

all: $(TARGET)

$(TARGET): snake.c
	$(CC) $(CFLAGS) -o $(TARGET) snake.c $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean
