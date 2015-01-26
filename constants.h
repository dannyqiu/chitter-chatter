#define SERVER_PORT 6666
static const char *SERVER_IP = "127.0.0.1";

#define HEADER_SIZE 10
#define MSG_SIZE 512
#define TIMESTAMP_SIZE 10 

/* Message types */
#define TYPE_CONTROL 0
#define TYPE_MESSAGE 1

struct chat_packet {
    int sequence;           // Packet number in sequence
    int total;              // Total number of packets in message
    int type;               // Type header
    char message[MSG_SIZE]; // Content
};

#define WINDOW_X_SIZE 1000
#define WINDOW_Y_SIZE 600
