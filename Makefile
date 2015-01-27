GTK_FLAGS=`pkg-config --cflags gtk+-3.0 gmodule-export-2.0`
GTK_LIBS=`pkg-config --libs gtk+-3.0`
FLAGS=-Wall -Wno-unused-variable -Wno-unused-function -rdynamic
CLIENT_FILES = gui.c gclient.c
SERVER_FILES = server.c
UTIL_FILES = util.c

all: setup server client

server: setup
	gcc $(FLAGS) -o build/server $(SERVER_FILES) $(UTIL_FILES)

client: setup
	gcc $(FLAGS) $(GTK_FLAGS) -o build/client $(CLIENT_FILES) $(UTIL_FILES) $(GTK_LIBS)

setup:
	mkdir -p build

clean:
	rm -rf build
	rm -rf profile
