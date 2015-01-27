#include "client.h"
#include "util.h"

int sock_fd; // Client sock to communicate with server
int client_id; // ID of client assigned by server

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

void cleanup() {
    remove_shared_memory();
}

int main() {
    signal(SIGINT, signal_handler);
    printf("Starting client...\n");

    client_id = connect_to_server(&sock_fd);

    if (init_shared_memory() < 0) {
        print_error("Problem creating shared memory");
    }

    int child_pid = fork();
    if (child_pid) { // Parent handles sending
        /*
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
                send_message_to_server(sock_fd, input, input_len);
            }
            free(input);
        }
        */
        send_create_channel_to_server(sock_fd, "CRAZYYYY");
        wait(NULL);
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
                if (package.type == TYPE_MESSAGE) {
                    char *recv_message = (char *) malloc((package.total+1) * MSG_SIZE * sizeof(char));
                    strncpy(recv_message, package.message, MSG_SIZE);
                    while (package.sequence < package.total) {
                        recv(sock_fd, &package, sizeof(struct chat_packet), 0);
                        strncpy(recv_message + (package.sequence * MSG_SIZE), package.message, MSG_SIZE);
                    }
                    printf("\nReceived: %s\n", recv_message);
                    free(recv_message);
                }
                else if (package.type == TYPE_JOIN_CHANNEL) {
                    change_current_channel(package.channel_id);
                    add_channel(package.channel_id);
                    printf("Joined channel %d\n", package.channel_id);
                }
                else if (package.type == TYPE_CREATE_CHANNEL) {
                    send_join_channel_to_server(sock_fd, package.channel_id); // This line is not required because creator automatically joins channel
                    change_current_channel(package.channel_id);
                    add_channel(package.channel_id);
                    printf("Created and changed to channel %d\n", package.channel_id);
                }
            }
        }
        cleanup();
    }

    return 0;
}

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

void send_message_to_server(int sock_fd, char *message, size_t message_len) {
    int num_packets = message_len / MSG_SIZE;
    int n;
    for (n=0; n<=num_packets; ++n) {
        struct chat_packet package;
        package.sequence = n;
        package.total = num_packets;
        package.type = TYPE_MESSAGE;
        package.client_id = client_id;
        package.channel_id = get_current_channel();
        strncpy(package.message, message + (n * MSG_SIZE), MSG_SIZE);
        send(sock_fd, &package, sizeof(struct chat_packet), 0);
    }
}

void send_join_channel_to_server(int sock_fd, int channel_id) {
    struct chat_packet package;
    package.sequence = 0;
    package.total = 0;
    package.type = TYPE_JOIN_CHANNEL;
    package.client_id = client_id;
    package.channel_id = channel_id;
    send(sock_fd, &package, sizeof(struct chat_packet), 0);
}

void send_create_channel_to_server(int sock_fd, char *channel_name) {
    struct chat_packet package;
    package.sequence = 0;
    package.total = 0;
    package.type = TYPE_CREATE_CHANNEL;
    package.client_id = client_id;
    package.channel_id = 0;
    strncpy(package.message, channel_name, CHANNEL_NAME_SIZE);
    send(sock_fd, &package, sizeof(struct chat_packet), 0);
}

int get_current_channel() {
    int current_channel = *(get_shared_memory());
    release_shared_memory();
    return current_channel;
}

void change_current_channel(int new_channel) {
    int *shm = get_shared_memory();
    if (shm == (void *) -1) {
        print_error("Problem accessing shared memory to change current channel");
    }
    *shm = new_channel;
    release_shared_memory();
}

// Output needs to be freed
int * get_channels() {
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
    int num_channels = 0;
    int *channel_ids = (int *) malloc(num_channels * sizeof(int));
    while ((token = strsep(&current_pos, "\n")) != NULL) {
        ++num_channels;
        channel_ids = (int *) realloc(channel_ids, num_channels * sizeof(int));
        channel_ids[num_channels-1] = atoi(token);
    }
    free(channels_string);
    return channel_ids;
}

void add_channel(int channel_id) {
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

int init_shared_memory() {
    key_t shmkey = ftok(SHM_KEY_FILE, KEY_ID);
    if (shmkey == -1) {
        return -1;
    }
    key_t semkey = ftok(SEM_KEY_FILE, KEY_ID);
    if (shmkey == -1) {
        return -1;
    }
    int shmid = shmget(shmkey, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        return -1;
    }
    int *shm = shmat(shmid, 0, 0);
    if (shm == (void *) -1) {
        return -1;
    }
    memset(shm, 0, sizeof(int)); // Zero the shared memory
    int semid = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        return -1;
    }
    union semun su;
    su.val = 1;
    if (semctl(semid, 0, SETVAL, su) == -1) {
        return -1;
    }
    return 0;
}

int remove_shared_memory() {
    key_t shmkey = ftok(SHM_KEY_FILE, KEY_ID);
    key_t semkey = ftok(SEM_KEY_FILE, KEY_ID);
    int shmid = shmget(shmkey, sizeof(int), 0);
    if (shmid != -1) { // Test for shared memory existence
        if (shmctl(shmid, IPC_RMID, 0) == -1) {
            return 1;
        }
    }
    int semid = semget(semkey, 1, 0);
    if (semid != -1) { // Test for semaphore existence
        if (semctl(semid, 0, IPC_RMID) == -1) {
            return 1;
        }
    }
    return 0;
}

int * get_shared_memory() {
    key_t shmkey = ftok(SHM_KEY_FILE, KEY_ID);
    int shmid = shmget(shmkey, sizeof(int), 0);
    if (shmid == -1) {
        return (void *) -1;
    }
    int *shm = shmat(shmid, 0, 0);
    if (shm == (void *) -1) {
        return (void *) -1;
    }
    key_t semkey = ftok(SEM_KEY_FILE, KEY_ID);
    int semid = semget(semkey, 1, 0);
    if (semid == -1) {
        return (void *) -1;
    }
    if (semctl(semid, 0, GETVAL) <= 0) {
        printf("Waiting for the semamphore to be released...\n");
    }
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    semop(semid, &sb, 1);
    return shm;
}

int release_shared_memory() {
    key_t semkey = ftok(SEM_KEY_FILE, KEY_ID);
    int semid = semget(semkey, 1, 0);
    if (semid == -1) {
        return -1;
    }
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    semop(semid, &sb, 1);
    return 0;
}
