GTK_FLAGS=`pkg-config --cflags gtk+-3.0`
GTK_LIBS=`pkg-config --libs gtk+-3.0`
FLAGS=-Wall -Wno-unused-variable -Wno-unused-function
UTIL_FILES = util.c

test: clean test.c
	mkdir build
	gcc $(FLAGS) $(GTK_FLAGS) -o build/test test.c $(GTK_LIBS)

clean:
	rm -rf build
