#include "server.h"
#include "util.h"

int listen_sock, cli_sock; // Server listen sock, New client sock

struct client **client_list;
int num_clients;
struct channel **channel_list;
int num_channels;

static void signal_handler(int signo) {
    switch (signo) {
        case SIGINT:
            shutdown(listen_sock, SHUT_RDWR);
            close(listen_sock);
            cleanup();
            exit(0);
        case SIGSEGV:
            shutdown(listen_sock, SHUT_RDWR);
            close(listen_sock);
            exit(1);
    }
}

void cleanup() {
    for (; num_clients > 0; --num_clients) {
        struct client *cur_client = client_list[num_clients-1];
        if (cur_client) {
            free(cur_client);
        }
    }
    free(client_list);
    for (; num_channels > 0; --num_channels) {
        struct channel *cur_channel = channel_list[num_channels-1];
        if (cur_channel) {
            free(cur_channel);
        }
    }
    free(channel_list);
}

int add_client(int fd) {
    int client_id = rand();
    while (is_client_id_taken(client_id)) {
        client_id = rand();
    }
    ++num_clients;
    client_list = (struct client **) realloc(client_list, num_clients * sizeof(struct client *));
    struct client *new_client = (struct client *) malloc(sizeof(struct client));
    new_client->cli_sock = fd;
    new_client->cli_id = client_id;
    new_client->num_channels = 0;
    new_client->channels = (struct channel **) malloc(new_client->num_channels * sizeof(struct channel *));
    client_list[num_clients-1] = new_client;
    return client_id;
}

void remove_client(int fd) {
    int i;
    for (i=0; i<num_clients; ++i) {
        struct client *cur_client = client_list[i];
        if (cur_client) {
            if (cur_client->cli_sock == fd) {
                free(cur_client);
                client_list[i] = NULL;
            }
        }
    }
}

int is_client_id_taken(int client_id) {
    int i;
    for (i=0; i<num_clients; ++i) {
        struct client *cur_client = client_list[i];
        if (cur_client) {
            if (cur_client->cli_id == client_id) {
                return 1;
            }
        }
    }
    return 0;
}

struct client * get_client_by_sock(int sock_fd) {
    int i;
    for (i=0; i<num_clients; ++i) {
        struct client *cur_client = client_list[i];
        if (cur_client) {
            if (cur_client->cli_sock == sock_fd) {
                return cur_client;
            }
        }
    }
    return 0;
}

int add_channel_by_name(char *channel_name) {
    int channel_id = rand();
    while (is_channel_id_taken(channel_id)) {
        channel_id = rand();
    }
    return add_channel(channel_id, channel_name);
}

int add_channel(int channel_id, char *channel_name) {
    ++num_channels;
    channel_list = (struct channel **) realloc(channel_list, num_channels * sizeof(struct channel *));
    struct channel *new_channel = (struct channel *) malloc(sizeof(struct channel));
    new_channel->channel_id = channel_id;
    strncpy(new_channel->channel_name, channel_name, CHANNEL_NAME_SIZE);
    new_channel->num_clients = 0;
    new_channel->cli_ids = (int *) malloc(num_clients * sizeof(int));
    channel_list[num_channels-1] = new_channel;
    return channel_id;
}

int is_channel_id_taken(int channel_id) {
    struct channel *cur_channel = get_channel_by_id(channel_id);
    if (cur_channel) {
        return 1;
    }
    else {
        return 0;
    }
}

void add_client_to_channel(int client_id, int channel_id) {
    struct channel *cur_channel = get_channel_by_id(channel_id);
    if (cur_channel) {
        ++(cur_channel->num_clients);
        cur_channel->cli_ids = (int *) realloc(cur_channel->cli_ids, cur_channel->num_clients * sizeof(int));
        cur_channel->cli_ids[num_clients-1] = client_id;
    }
    else {
        print_error("Attempted to add client to invalid channel");
    }
}

int is_client_in_channel(int client_id, int channel_id) {
    struct channel *cur_channel = get_channel_by_id(channel_id);
    if (cur_channel) {
        int i;
        for (i=0; i<cur_channel->num_clients; ++i) {
            if (cur_channel->cli_ids[i] == client_id) {
                return 1;
            }
        }
        return 0;
    }
    else {
        print_error("Invalid channel to search for client");
        return 1;
    }
}

struct channel * get_channel_by_id(int channel_id) {
    int i;
    for (i=0; i<num_channels; ++i) {
        struct channel *cur_channel = channel_list[i];
        if (cur_channel) {
            if (cur_channel->channel_id == channel_id) {
                return cur_channel;
            }
        }
    }
    print_error("Channel not found!");
    return NULL;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGSEGV, signal_handler);
    struct sockaddr_in serv_addr, cli_addr; // Server address, Client address
    socklen_t cli_len;                      // Client address length
    fd_set masterfds, readfds;              // FD sets for master, read
    int fdmax;                              // Max FD for select()
    printf("Starting server...\n");

    srand(time(NULL)); // Seed random number generator

    /* Initialize client list */
    num_clients = 0;
    client_list = (struct client **) malloc(num_clients * sizeof(struct client *));
    num_channels = 0;
    channel_list = (struct channel **) malloc(num_channels * sizeof(struct channel *));

    /* Create socket */
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        print_error("Problem creating sock");
        exit(1);
    }

    /* Bind socket to port */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        print_error("Problem binding sock");
        exit(1);
    }

    /* Listen for connections */
    if (listen(listen_sock, 10) < 0) {
        printf("Problem listening on port %d\n", SERVER_PORT);
    }
    else {
        printf("Listening on port %d\n", SERVER_PORT);
    }

    FD_ZERO(&masterfds); // Clear master and read sets
    FD_ZERO(&readfds);
    FD_SET(listen_sock, &masterfds); // Add the listening sock to master
    fdmax = listen_sock; // Listening sock is currently highest FD

    /* Add initial master channel that every client defaults to */
    add_channel(MASTER_CHANNEL, "Master Channel");

    while (1) {
        readfds = masterfds;
        if (select(fdmax+1, &readfds, NULL, NULL, NULL) < 0) {
            print_error("Problem with selecting readable file descriptors");
            continue; // Repeat while loop without going through rest of code
        }

        int currentfd; // Loop through socket FD
        for (currentfd=0; currentfd<=fdmax; ++currentfd) {
            if (FD_ISSET(currentfd, &readfds)) { // FD that we can read from
                if (currentfd == listen_sock) {

                    /* Accept connection from new client */
                    cli_len = sizeof(cli_addr);
                    cli_sock = accept(listen_sock, (struct sockaddr *) &cli_addr, &cli_len);
                    if (cli_sock < 0) {
                        print_error("Problem accepting connection");
                    }
                    else {
                        FD_SET(cli_sock, &masterfds);
                        if (cli_sock > fdmax) {
                            fdmax = cli_sock;
                        }
                        char ip_buffer[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(cli_addr.sin_addr), ip_buffer, INET_ADDRSTRLEN);
                        int client_id = add_client(cli_sock);
                        send(cli_sock, &client_id, sizeof(int), 0);
                        printf("Connected to %s on %d [ID: %d]\n", ip_buffer, cli_sock, client_id);
                    }
        
                }
                else {

                    /* Handle data from connected clients */
                    struct chat_packet initial_package;
                    int nbytes = recv(currentfd, &initial_package, sizeof(struct chat_packet), 0);
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            printf("Connection closed by %d\n", currentfd);
                        }
                        else {
                            print_error("Problem receiving data from client");
                        }
                        close(currentfd);
                        FD_CLR(currentfd, &masterfds);
                        remove_client(currentfd);
                    }
                    else {
                        if (initial_package.type == TYPE_MESSAGE) {
                            char *recv_message = receive_message_from_client(currentfd, initial_package);
                            char *send_msg = recv_message; // Forward received data to all other clients. TODO: Implement channels
                            int recipient;
                            for (recipient=0; recipient<=fdmax; ++recipient) {
                                if (FD_ISSET(recipient, &masterfds) && recipient != currentfd && recipient != listen_sock) {
                                    send_message_to_client(recipient, TYPE_MESSAGE, recv_message, initial_package.client_id, initial_package.channel_id);
                                }
                            }
                            free(recv_message);
                        }
                        else if (initial_package.type == TYPE_JOIN_CHANNEL) {
                            add_client_to_channel(initial_package.client_id, initial_package.channel_id);
                            printf("Client %d has joined channel %d\n", initial_package.client_id, initial_package.channel_id);
                            send(currentfd, &initial_package, sizeof(struct chat_packet), 0); // Server sends acknowledgement of join
                        }
                        else if (initial_package.type == TYPE_CREATE_CHANNEL) {
                            int channel_id = add_channel_by_name(initial_package.message);
                            add_client_to_channel(initial_package.client_id, channel_id);
                            printf("Channel %s created with ID %d by Client %d\n", initial_package.message, channel_id, initial_package.client_id);
                            initial_package.channel_id = channel_id;
                            send(currentfd, &initial_package, sizeof(struct chat_packet), 0); // Server sends channel id back
                            char *channels_string = build_channels_list_for_client();
                            //printf("Sending to every client: %s\n", channels_string);
                            int recipient;
                            for (recipient=0; recipient<=fdmax; ++recipient) {
                                if (FD_ISSET(recipient, &masterfds) && recipient != listen_sock) {
                                    send_message_to_client(recipient, TYPE_GET_CHANNELS, channels_string, 0, 0); // Server updates all others clients with channel listing
                                }
                            }
                            free(channels_string);
                        }
                        else if (initial_package.type == TYPE_GET_CHANNELS) {
                            char *channels_string = build_channels_list_for_client();
                            send_message_to_client(currentfd, TYPE_GET_CHANNELS, channels_string, 0, 0);
                            free(channels_string);
                        }
                    }

                }
            }
        }
    } // WOW SUCH NESTING

    return 0;
}

// Output needs to be freed
char * receive_message_from_client(int listen_fd, struct chat_packet package) {
    printf("[RECEIVED FROM SOCK %d - CLIENT %d - CHANNEL %d] (%d of %d) [TYPE %d]: %s\n", listen_fd, package.client_id, package.channel_id, package.sequence, package.total, package.type, package.message);
    char *recv_message = (char *) malloc((package.total+1) * MSG_SIZE * sizeof(char));
    strncpy(recv_message, package.message, MSG_SIZE);
    while (package.sequence < package.total) {
        recv(listen_fd, &package, sizeof(struct chat_packet), 0);
        strncpy(recv_message + (package.sequence * MSG_SIZE), package.message, MSG_SIZE);
        printf("[RECEIVED FROM SOCK %d - CLIENT %d - CHANNEL %d] (%d of %d) [TYPE %d]: %s\n", listen_fd, package.client_id, package.channel_id, package.sequence, package.total, package.type, package.message);
    }
    //printf("[COMBINED MESSAGE FROM %d]: %s\n", listen_fd, recv_message);
    return recv_message;
}

void send_message_to_client(int sock_fd, int type, char *message, int client_id, int channel_id) {
    int num_packets = strlen(message) / MSG_SIZE;
    int n;
    for (n=0; n<=num_packets; ++n) {
        struct chat_packet package;
        package.sequence = n;
        package.total = num_packets;
        package.type = type;
        package.client_id = client_id;
        package.channel_id = channel_id;
        strncpy(package.message, message + (n * MSG_SIZE), MSG_SIZE);
        send(sock_fd, &package, sizeof(struct chat_packet), 0);
    }
}

char * build_channels_list_for_client() {
    char buffer[16]; // Enough to hold ascii int + ','
    int channels_string_len = 0;
    int channels_string_len_max = CHANNEL_STRING_LEN_INCREMENT;
    char *channels_string = (char *) malloc(channels_string_len_max);
    int i;
    for (i=0; i<num_channels; ++i) {
        snprintf(buffer, sizeof(buffer), "%d,", channel_list[i]->channel_id);
        if (channels_string_len + strlen(buffer) + 2 >= channels_string_len_max) {
            channels_string_len_max += CHANNEL_STRING_LEN_INCREMENT;
            channels_string = (char *) realloc(channels_string, channels_string_len_max * sizeof(char));
        }
        strncpy(channels_string + channels_string_len, buffer, strlen(buffer));
        channels_string_len += strlen(buffer);
    }
    return channels_string;
}
