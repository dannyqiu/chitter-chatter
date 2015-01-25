#include "server.h"
#include "util.h"
#include "constants.h"

int main() {
    struct sockaddr_in serv_addr, cli_addr; // Server address, Client address
    socklen_t cli_len;                      // Client address length
    int listen_sock, cli_sock;              // Server listen sock, New client sock
    fd_set masterfds, readfds;              // FD sets for master, read
    int fdmax;                              // Max FD for select()
    printf("Starting server...\n");

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
                        printf("Connected to %s\n", ip_buffer);
                    }
        
                }
                else {

                    /* Get data from client and forward to all other clients */
                    char buffer[1024];
                    int nbytes = recv(currentfd, buffer, sizeof(buffer), 0);
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
                        printf("[RECEIVED FROM %d]: %s", currentfd, buffer);
                        int recipient;
                        for (recipient=0; recipient<=fdmax; ++recipient) {
                            // Uncomment the below line when implemented. We don't want clients having an echo??
                            //if (FD_ISSET(recipient, &masterfds) && recipient != currentfd && recipient != listen_sock) {
                            if (FD_ISSET(recipient, &masterfds) && recipient != listen_sock) {
                                if (send(recipient, buffer, sizeof(buffer), 0) < 0) {
                                    print_error("Problem sending data to client");
                                }
                            }
                        }
                    }

                }
            }
        }
    } // WOW SUCH NESTING

    return 0;
}
