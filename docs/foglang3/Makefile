CC = gcc
CFLAGS = -lm -Wno-switch
SFLAGS = -fsanitize=address -static-libasan -g
TARGET = build/foglang3

$(TARGET): main.c foglang_ast.c foglang_debug.c foglang_utils.c foglang.h
	$(CC) -o $(TARGET) main.c $(CFLAGS)

clean:
	rm -f $(TARGET)

sanitize:
	$(CC) -o $(TARGET) main.c $(CFLAGS) $(SFLAGS)