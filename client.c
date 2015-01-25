#include "client.h"
#include "util.h"
#include "constants.h"

int main() {
    struct sockaddr_in cli_addr;
    int sock_fd;
    printf("Starting client...\n");

    /* Create socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
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

    int child_pid = fork();
    if (child_pid) { // Parent handles sending
        char buffer[MSG_SIZE];
        while (1) {
            printf("Enter a message: ");
            fflush(stdout);
            fgets(buffer, sizeof(buffer), stdin);
            if (feof(stdin)) {
                kill(child_pid, SIGKILL);
                exit(0);
            }
            else {
                send(sock_fd, buffer, sizeof(buffer), 0);
            }
        }
    }
    else { // Child handles receiving
        char buffer[MSG_SIZE];
        int nbytes;
        while (1) {
            nbytes = recv(sock_fd, buffer, sizeof(buffer), 0);
            if (nbytes <= 0) {
                if (nbytes == 0) {
                    printf("Connection closed by the server :(");
                }
                else {
                    print_error("Problem receiving data from the server");
                }
                kill(getppid(), SIGPIPE);
                exit(1);
            }
            else {
                printf("\nReceived: %s", buffer);
            }
        }
    }

    return 0;
}
