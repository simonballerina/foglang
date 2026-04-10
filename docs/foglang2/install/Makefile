CC = gcc
CFLAGS = -lm
TARGET = build/foglang2

$(TARGET): main.c foglang.h foglang_eval.c foglang_var.c
	$(CC) -o $(TARGET) main.c $(CFLAGS)

clean:
	rm -f $(TARGET)