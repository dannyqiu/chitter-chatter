GTK_FLAGS=`pkg-config --cflags gtk+-3.0`
GTK_LIBS=`pkg-config --libs gtk+-3.0`
FLAGS=-Wall

test: test.c
	gcc $(GTK_FLAGS) -o test test.c $(GTK_LIBS)
