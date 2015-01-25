#include "server.h"
#include "util.h"
#include "constants.h"

int main() {
    struct sockaddr_in serv_addr, cli_addr;
    printf("Starting server...\n");

    /* Create socket */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        print_error("Problem creating sock");
        exit(1);
    }

    /* Bind socket to port */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        print_error("Problem binding sock");
        exit(1);
    }

    /* Listen for connections */
    if (listen(sock_fd, 3) < 0) {
        printf("Problem listening on port %d\n", SERVER_PORT);
    }
    else {
        printf("Listening on port %d\n", SERVER_PORT);
    }

    /* Accept connection */
    socklen_t cli_len = sizeof(cli_addr);
    int cli_sock = accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_len);
    if (cli_sock < 0) {
        print_error("Problem accepting connection");
    }

    char buffer[1024];
    read(cli_sock, buffer, sizeof(buffer));
    printf("Read: %s\n", buffer);

    return 0;
}
