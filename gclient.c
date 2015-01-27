#include "gclient.h"
#include "util.h"

/* Used when client is running indepent of gui */
/*
int sock_fd; // Client sock to communicate with server
int client_id; // ID of client assigned by server
int channel_id; // ID of channel client is on

static void signal_handler(int signo) {
    switch (signo) {
        case SIGINT:
            close(sock_fd);
            cleanup();
            exit(0);
        case SIGPIPE:
            close(sock_fd);
            cleanup();
            exit(1);
    }
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, signal_handler);
    printf("Starting client...\n");

    client_id = connect_to_server(&sock_fd);

    int child_pid = fork();
    if (child_pid) { // Parent handles sending
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
                send_message_to_server(sock_fd, client_id, channel_id, input);
            }
            free(input);
        }
    }
    else { // Child handles receiving
        while (1) {
            struct chat_packet package;
            int nbytes = recv(sock_fd, &package, sizeof(struct chat_packet), 0);
            if (nbytes <= 0) {
                if (nbytes == 0) {
                    printf("Connection closed by the server :(\n");
                }
                else {
                    print_error("Problem receiving data from the server");
                }
                kill(getppid(), SIGPIPE);
                exit(1);
            }
            else {
               char *recv_message = (char *) malloc((package.total+1) * MSG_SIZE * sizeof(char));
                strncpy(recv_message, package.message, MSG_SIZE);
                while (package.sequence < package.total) {
                    recv(client_sock, &package, sizeof(struct chat_packet), 0);
                    strncpy(recv_message + (package.sequence * MSG_SIZE), package.message, MSG_SIZE);
                }
                g_print("\nReceived: %s\n", recv_message);
                free(recv_message);
            }
        }
        cleanup();
    }
    return 0;
}
*/

int connect_to_server(int *sock_fd) {
    struct sockaddr_in cli_addr;

    /* Create socket */
    *sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sock_fd < 0) {
        print_error("Problem creating sock");
        exit(1);
    }

    /* Connect socket to server */
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(SERVER_PORT);
    inet_aton(SERVER_IP, &(cli_addr.sin_addr));
    if (connect(*sock_fd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        print_error("Problem connecting to server");
        exit(1);
    }

    int client_id_from_server;
    if (recv(*sock_fd, &client_id_from_server, sizeof(int), 0) < 0) {
        print_error("Problem getting client ID from server");
    }
    else {
        printf("Got client ID of %d from server\n", client_id_from_server);
    }
    return client_id_from_server;
}

void send_message_to_server(int sock_fd, int client_id, int channel_id, char *message) {
    int num_packets = strlen(message) / MSG_SIZE;
    int n;
    for (n=0; n<=num_packets; ++n) {
        struct chat_packet package;
        package.sequence = n;
        package.total = num_packets;
        package.type = TYPE_MESSAGE;
        package.client_id = client_id;
        package.channel_id = channel_id;
        strncpy(package.message, message + (n * MSG_SIZE), MSG_SIZE);
        send(sock_fd, &package, sizeof(struct chat_packet), 0);
    }
}

void send_join_channel_to_server(int sock_fd, int client_id, int channel_id) {
    struct chat_packet package;
    package.sequence = 0;
    package.total = 0;
    package.type = TYPE_JOIN_CHANNEL;
    package.client_id = client_id;
    package.channel_id = channel_id;
    send(sock_fd, &package, sizeof(struct chat_packet), 0);
}

void send_create_channel_to_server(int sock_fd, int client_id, char *channel_name) {
    struct chat_packet package;
    package.sequence = 0;
    package.total = 0;
    package.type = TYPE_CREATE_CHANNEL;
    package.client_id = client_id;
    package.channel_id = 0;
    strncpy(package.message, channel_name, CHANNEL_NAME_SIZE);
    send(sock_fd, &package, sizeof(struct chat_packet), 0);
}

void send_get_channels_to_server(int sock_fd) {
    struct chat_packet package;
    package.sequence = 0;
    package.total = 0;
    package.type = TYPE_GET_CHANNELS;
    send(sock_fd, &package, sizeof(struct chat_packet), 0);
}

// Output needs to be freed
int * get_channels(int client_id) {
    if (access(PROFILE_FOLDER, F_OK) < 0) { // Folder does not exist
        if (mkdir(PROFILE_FOLDER, 0755) < 0) {
            print_error("Problem creating directory");
        }
    }
    char filename[PROFILE_PATH_SIZE];
    snprintf(filename, sizeof(filename), "%s/%d", PROFILE_FOLDER, client_id);
    int profilefd = open(filename, O_RDONLY | O_CREAT, 0666);
    int file_size = lseek(profilefd, 0, SEEK_END) - lseek(profilefd, 0, SEEK_SET);
    char *channels_string = (char *) malloc(file_size * sizeof(char));
    read(profilefd, channels_string, file_size);
    close(profilefd);
    char *token, *current_pos;
    current_pos = channels_string;
    int num_channels = 0;
    int *channel_ids = (int *) malloc(num_channels * sizeof(int));
    while ((token = strsep(&current_pos, "\n")) != NULL) {
        ++num_channels;
        channel_ids = (int *) realloc(channel_ids, num_channels * sizeof(int));
        channel_ids[num_channels-1] = atoi(token);
    }
    /* Add NULL to end */
    ++num_channels;
    channel_ids = (int *) realloc(channel_ids, num_channels * sizeof(int));
    channel_ids[num_channels-1] = -1;
    free(channels_string);
    return channel_ids;
}

void add_channel(int client_id, int channel_id) {
    if (access(PROFILE_FOLDER, F_OK) < 0) { // Folder does not exist
        if (mkdir(PROFILE_FOLDER, 0755) < 0) {
            print_error("Problem creating directory");
        }
    }
    char filename[PROFILE_PATH_SIZE];
    snprintf(filename, sizeof(filename), "%s/%d", PROFILE_FOLDER, client_id);
    int profilefd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (profilefd < 0) {
        print_error("Problem with opening profile file for writing");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d\n", channel_id);
    if (write(profilefd, buffer, strlen(buffer)) < 0) {
        print_error("Problem with writing to profile file");
    }
    close(profilefd);
}

int is_channel_in_client_channels(int client_id, int channel_id) {
    int *client_channel_ids = get_channels(client_id);
    int i;
    for (i=0; client_channel_ids[i] != -1; ++i) {
        if (client_channel_ids[i] == channel_id) {
            free(client_channel_ids);
            return 1;
        }
    }
    free(client_channel_ids);
    return 0;
}
