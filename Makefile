GTK_FLAGS=`pkg-config --cflags gtk+-3.0`
GTK_LIBS=`pkg-config --libs gtk+-3.0`
FLAGS=-Wall -Wno-unused-variable -Wno-unused-function
UTIL_FILES = util.c

test: clean test.c
	mkdir -p build
	gcc $(FLAGS) $(GTK_FLAGS) -o build/test test.c $(GTK_LIBS)

all: setup server

server: server.h server.c
	gcc $(FLAGS) -o build/server server.c $(UTIL_FILES)

setup:
	mkdir -p build

clean:
	rm -rf build
