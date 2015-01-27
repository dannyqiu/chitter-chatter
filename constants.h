#define SERVER_PORT 6666
static const char *SERVER_IP = "127.0.0.1";

#define PROFILE_PATH_SIZE 32
static const char *PROFILE_FOLDER = "profile";

#define TIMESTAMP_SIZE 10 
#define MSG_SIZE 512
#define CHANNEL_NAME_SIZE 256
#define DISPLAY_NAME_SIZE 128

#define MASTER_CHANNEL 99

/* Message types */
#define TYPE_MESSAGE 0
#define TYPE_JOIN_CHANNEL 1
#define TYPE_CREATE_CHANNEL 2
#define TYPE_GET_CHANNELS 3

struct chat_packet {
    int sequence;           // Packet number in sequence
    int total;              // Total number of packets in message
    int type;               // Type header
    int client_id;          // ID of client that sent packet
    int channel_id;         // ID of channel that packet belongs to
    char message[MSG_SIZE]; // Content
};

#define WINDOW_X_SIZE 1000
#define WINDOW_Y_SIZE 600
