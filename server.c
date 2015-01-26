#include "server.h"
#include "util.h"
#include "constants.h"

int listen_sock, cli_sock; // Server listen sock, New client sock

struct client **client_list;
int num_clients;

static void signal_handler(int signo) {
    switch (signo) {
        case SIGINT:
            shutdown(listen_sock, SHUT_RDWR);
            close(listen_sock);
            cleanup();
            exit(0);
    }
}

void cleanup() {
    for (; num_clients > 0; --num_clients) {
        free(client_list[num_clients-1]);
    }
    free(client_list);
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
    new_client->channels = NULL;
    client_list[num_clients-1] = new_client;
    return client_id;
}

int is_client_id_taken(int client_id) {
    int i;
    for (i=0; i<num_clients; ++i) {
        if (client_list[i]->cli_id == client_id) {
            return 1;
        }
    }
    return 0;
}

int main() {
    signal(SIGINT, signal_handler);
    struct sockaddr_in serv_addr, cli_addr; // Server address, Client address
    socklen_t cli_len;                      // Client address length
    fd_set masterfds, readfds;              // FD sets for master, read
    int fdmax;                              // Max FD for select()
    printf("Starting server...\n");

    srand(time(NULL)); // Seed random number generator

    /* Initialize client list */
    num_clients = 0;
    client_list = (struct client **) malloc(num_clients * sizeof(struct client *));

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
                    struct chat_packet package;
                    int nbytes = recv(currentfd, &package, sizeof(package), 0);
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            printf("Connection closed by %d\n", currentfd);
                        }
                        else {
                            print_error("Problem receiving data from client");
                        }
                        close(currentfd);
                        FD_CLR(currentfd, &masterfds);
                    }
                    else {
                        printf("[RECEIVED FROM SOCK %d - CLIENT %d - CHANNEL %d] (%d of %d) [TYPE %d]: %s\n", currentfd, package.client_id, package.channel_id, package.sequence, package.total, package.type, package.message);
                        char *recv_message = (char *) malloc(package.total * MSG_SIZE * sizeof(char));
                        strncpy(recv_message, package.message, MSG_SIZE);
                        while (package.sequence < package.total) {
                            recv(currentfd, &package, sizeof(package), 0);
                            strncpy(recv_message + (package.sequence * MSG_SIZE), package.message, MSG_SIZE);
                            printf("[RECEIVED FROM SOCK %d - CLIENT %d - CHANNEL %d] (%d of %d) [TYPE %d]: %s\n", currentfd, package.client_id, package.channel_id, package.sequence, package.total, package.type, package.message);
                        }
                        printf("[COMBINED MESSAGE FROM %d]: %s\n", currentfd, recv_message);
                        char *send_msg = recv_message; // Forward received data to all other clients. TODO: Implement channels
                        int recipient;
                        for (recipient=0; recipient<=fdmax; ++recipient) {
                            if (FD_ISSET(recipient, &masterfds) && recipient != currentfd && recipient != listen_sock) {
                                int num_packets = strlen(recv_message) / MSG_SIZE;
                                int n;
                                for (n=0; n<=num_packets; ++n) {
                                    struct chat_packet package;
                                    package.sequence = n;
                                    package.total = num_packets;
                                    package.type = TYPE_MESSAGE;
                                    strncpy(package.message, send_msg + (n * MSG_SIZE), MSG_SIZE);
                                    send(recipient, &package, sizeof(package), 0);
                                }
                            }
                        }
                        free(recv_message);
                    }

                }
            }
        }
    } // WOW SUCH NESTING

    return 0;
}
