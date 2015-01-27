#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "constants.h"

void cleanup();

int connect_to_server(int *);
void send_message_to_server(int, int, int, char *);
void send_join_channel_to_server(int, int, int);
void send_create_channel_to_server(int, int, char *);
void send_get_channels_to_server(int);

int * get_channels();
void add_channel(int);
