GTK_FLAGS=`pkg-config --cflags gtk+-3.0 gmodule-export-2.0`
GTK_LIBS=`pkg-config --libs gtk+-3.0`
FLAGS=-Wall -Wno-unused-variable -Wno-unused-function -rdynamic
UTIL_FILES = util.c

gui: clean gui.c
	mkdir -p build
	gcc $(FLAGS) $(GTK_FLAGS) -o build/gui gui.c $(GTK_LIBS)

all: setup server client

server: server.h server.c
	gcc $(FLAGS) -o build/server server.c $(UTIL_FILES)

client: client.h client.c
	gcc $(FLAGS) -o build/client client.c $(UTIL_FILES)

setup:
	mkdir -p build

clean:
	rm -rf build
