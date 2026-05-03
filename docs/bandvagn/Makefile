CC = gcc
CFLAGS = -lcurl 
TARGET = build/vagn

$(TARGET): main.c bandvagn.h http.c
	$(CC) -o $(TARGET) main.c $(CFLAGS)

clean:
	rm -f $(TARGET)