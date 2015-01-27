#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "constants.h"

#define CHANNEL_STRING_LEN_INCREMENT 256

struct channel {
    int channel_id;     // ID of the channel
    char channel_name[CHANNEL_NAME_SIZE]; // Name of the channel
    int num_clients;    // Counter for clients in channel
    int *cli_ids;       // Array pointer to clients in channel
};

struct client {
    int cli_sock;               // FD for client
    int cli_id;                 // Unique ID for client
    int num_channels;           // Counter for client channels
    struct channel **channels;  // Array pointer to client channels
};

void cleanup();

int add_client(int);
void remove_client(int);
int is_client_id_taken(int);
struct client * get_client_by_sock(int);

int add_channel(int, char *);
int add_channel_by_name(char *);
int is_channel_id_taken(int);
void add_client_to_channel(int, int);
int is_client_in_channel(int, int);
struct channel * get_channel_by_id(int);
char * build_channels_list_for_client();

char * receive_message_from_client(int, struct chat_packet);
void send_message_to_client(int, int, char *, int, int);

