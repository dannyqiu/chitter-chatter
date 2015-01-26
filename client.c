#include "client.h"
#include "util.h"
#include "constants.h"

int sock_fd; // Client sock to communicate with server
int client_id; // ID of client assigned by server
int *channel_ids; // ID of channels client is on
int num_channels;
int current_channel;

static void signal_handler(int signo) {
    switch (signo) {
        case SIGINT:
            close(sock_fd);
            exit(0);
    }
}

void cleanup() {
    free(channel_ids);
}

void add_channel(int channel_id) {
    ++num_channels;
    channel_ids = (int *) realloc(channel_ids, num_channels * sizeof(int));
    channel_ids[num_channels-1] = channel_id;
}

int main() {
    signal(SIGINT, signal_handler);
    struct sockaddr_in cli_addr;
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

    if (recv(sock_fd, &client_id, sizeof(int), 0) < 0) {
        print_error("Problem getting client ID from server");
    }
    else {
        printf("Got client ID of %d from server\n", client_id);
    }

    int child_pid = fork();
    if (child_pid) { // Parent handles sending
        char buffer[MSG_SIZE];
        while (1) {
            printf("Enter a message: ");
            fflush(stdout);
            size_t input_len;
            char *input_temp = fgetln(stdin, &input_len);
            char *input = (char *) malloc(input_len * sizeof(char));
            strncpy(input, input_temp, input_len);
            input[input_len-1] = '\0';
            if (feof(stdin)) {
                kill(child_pid, SIGKILL);
                exit(0);
            }
            else {
                int num_packets = input_len / MSG_SIZE;
                int n;
                for (n=0; n<=num_packets; ++n) {
                    struct chat_packet package;
                    package.sequence = n;
                    package.total = num_packets;
                    package.type = TYPE_MESSAGE;
                    package.client_id = client_id;
                    package.channel_id = current_channel;
                    strncpy(package.message, input + (n * MSG_SIZE), MSG_SIZE);
                    send(sock_fd, &package, sizeof(package), 0);
                }
            }
            free(input);
        }
    }
    else { // Child handles receiving
        while (1) {
            struct chat_packet package;
            int nbytes = recv(sock_fd, &package, sizeof(package), 0);
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
                char *recv_message = (char *) malloc(package.total * MSG_SIZE * sizeof(char));
                strncpy(recv_message, package.message, MSG_SIZE);
                while (package.sequence < package.total) {
                    recv(sock_fd, &package, sizeof(package), 0);
                    strncpy(recv_message + (package.sequence * MSG_SIZE), package.message, MSG_SIZE);
                }
                printf("\nReceived: %s\n", recv_message);
            }
        }
    }

    return 0;
}
