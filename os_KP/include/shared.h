#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define MAX_PLAYERS 4
#define MAX_WORD_LEN 20
#define MAX_NAME_LEN 50
#define PIPE_SERVER_NAME "/tmp/bulls_cows_server"
#define PIPE_CLIENT_TEMPLATE "/tmp/bulls_cows_client_%d"

#define BUFFER_SIZE 256
#define PIPE_NAME_LENGTH 50

#ifndef DEBUG
#define DEBUG 0
#endif

#define LOG_FILE "server.log"

#define LOG_ERROR(fmt, ...) \
    do { \
        FILE *log_fp = fopen(LOG_FILE, "a"); \
        if (log_fp) { \
            fprintf(log_fp, "[ERROR] " fmt "\n", ##__VA_ARGS__); \
            fclose(log_fp); \
        } \
        fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__); \
    } while(0)

#define LOG_INFO(fmt, ...) \
    do { \
        if (DEBUG) { \
            FILE *log_fp = fopen(LOG_FILE, "a"); \
            if (log_fp) { \
                fprintf(log_fp, "[INFO] " fmt "\n", ##__VA_ARGS__); \
                fclose(log_fp); \
            } \
            fprintf(stderr, "[INFO] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)

#define LOG_DEBUG(fmt, ...) \
    do { \
        if (DEBUG) { \
            FILE *log_fp = fopen(LOG_FILE, "a"); \
            if (log_fp) { \
                fprintf(log_fp, "[DEBUG] " fmt "\n", ##__VA_ARGS__); \
                fclose(log_fp); \
            } \
            fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)

typedef enum {
    MSG_CONNECT,
    MSG_GUESS,
    MSG_RESULT,
    MSG_GAME_STATE,
    MSG_DISCONNECT,
    MSG_WIN
} MessageType;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int active;
    char pipe_name[PIPE_NAME_LENGTH];
} Player;

typedef struct {
    char secret_word[MAX_WORD_LEN];
    Player players[MAX_PLAYERS];
    int player_count;
    int game_active;
} Game;

typedef struct {
    MessageType type;
    char sender[MAX_NAME_LEN];
    char data[BUFFER_SIZE];
    int bulls;
    int cows;
    char message[BUFFER_SIZE];
} Message;

const char* message_type_to_string(MessageType type);
void create_message(Message *msg, MessageType type, const char *sender, const char *data);
void create_client_pipe_name(int client_id, char *pipe_name, size_t size);

#endif // SHARED_H

