#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "constants.h"

extern char *display_name;

void cleanup();

void add_channel(int);

void send_message_to_server(int, char *, size_t);
void send_join_channel_to_server();
void send_create_channel_to_server(int, char *);
