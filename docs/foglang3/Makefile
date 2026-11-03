CC = gcc
CFLAGS = -lm -Wno-switch
SFLAGS = -fsanitize=address -static-libasan -g
WFLAGS = -Wno-c23-extensions -Wno-enum-compare -Wno-implicit-enum-enum-cast

TARGET = build/foglang3
INSTALL_TARGET = /usr/local/bin/foglang3

$(TARGET): main.c foglang_ast.c foglang_var.c foglang_debug.c foglang_utils.c foglang.h
	mkdir -p build
	$(CC) -o $(TARGET) main.c $(CFLAGS) $(WFLAGS)

clean:
	rm -rf $(TARGET)

sanitize:
	$(CC) -o $(TARGET) main.c $(CFLAGS) $(SFLAGS) $(WFLAGS)

install:
	$(CC) -o $(INSTALL_TARGET) main.c $(CFLAGS) $(WFLAGS)
