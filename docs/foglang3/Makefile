CC = gcc
CFLAGS =
TARGET = build/foglang3

$(TARGET): main.c foglang_ast.c foglang_debug.c foglang_utils.c foglang.h
	$(CC) -o $(TARGET) main.c $(CFLAGS)

clean:
	rm -f $(TARGET)