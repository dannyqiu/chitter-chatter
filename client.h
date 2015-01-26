#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "constants.h"

extern char *display_name;

void cleanup();

void add_channel(int);

void send_message_to_server(int, char *, size_t);
void send_join_channel_to_server(int, int);
void send_create_channel_to_server(int, char *);

int get_current_channel();
void change_current_channel(int);


#define SHM_KEY_FILE "Makefile"
#define SEM_KEY_FILE "README.md"
#define KEY_ID 694

int init_shared_memory();
int remove_shared_memory();
int * get_shared_memory();
int release_shared_memory();
