CC = gcc
CFLAGS = -lcurl 
SFLAGS = -fsanitize=address -static-libasan -g
TARGET = build/foglang2_install

$(TARGET): main.c ../bandvagn/http.c
	$(CC) -o $(TARGET) main.c $(CFLAGS)

clean:
	rm -f $(TARGET)

sanitize:
	$(CC) -o $(TARGET) main.c $(CFLAGS) $(SFLAGS)