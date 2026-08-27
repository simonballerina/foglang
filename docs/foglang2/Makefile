CC = gcc
CFLAGS = -lm
SFLAGS = -fsanitize=address -static-libasan -g
TARGET = build/foglang2

$(TARGET): main.c foglang.h foglang_eval.c foglang_var.c
	$(CC) -o $(TARGET) main.c $(CFLAGS)

clean:
	rm -f $(TARGET)

sanitize:
	$(CC) -o $(TARGET) main.c $(CFLAGS) $(SFLAGS)