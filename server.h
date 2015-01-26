#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct channel {
    int *cli_ids;   // Array pointer to clients in channel
};

struct client {
    int cli_sock;               // FD for client
    int cli_id;                 // Unique ID for client
    struct channel *channels;    // Array pointer to client channels
};

void cleanup();

int add_client(int);
int is_client_id_taken(int);
