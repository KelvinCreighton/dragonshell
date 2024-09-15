CC = gcc
CFLAGS = -Wall -g
TARGET = dragonshell

$(TARGET): dragonshell.c
	clear
	$(CC) $(CFLAGS) -o $(TARGET) dragonshell.c 

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

remake:
	make clean
	make $(TARGET)

mrun:
	make remake
	make run

all: $(TARGET) run clean

