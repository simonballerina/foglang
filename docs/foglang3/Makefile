CC = gcc
CFLAGS =
TARGET = build/foglang3

$(TARGET): ast.c debug_print.c foglang_utils.c foglang.h
	$(CC) -o $(TARGET) ast.c $(CFLAGS)

clean:
	rm -f $(TARGET)