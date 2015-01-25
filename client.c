#include "client.h"
#include "util.h"
#include "constants.h"

int main() {
    struct sockaddr_in cli_addr;
    printf("Starting client...\n");

    /* Create socket */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        print_error("Problem creating sock");
        exit(1);
    }

    /* Connect socket to server */
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(SERVER_PORT);
    inet_aton(SERVER_IP, &(cli_addr.sin_addr));
    if (connect(sock_fd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        print_error("Problem connecting to server");
        exit(1);
    }

    while(1) {
        printf("Enter a message: ");
        fflush(stdout);
        char buffer[1024];
        fgets(buffer, sizeof(buffer), stdin);
        write(sock_fd, buffer, sizeof(buffer));
        read(sock_fd, buffer, sizeof(buffer));
        printf("Received: %s\n", buffer);
    }

    return 0;
}
