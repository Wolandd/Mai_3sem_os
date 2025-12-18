#include "shared.h"
#include <signal.h>
#include <sys/select.h>
#include <time.h>

static int client_id = 0;
static char client_name[MAX_NAME_LEN] = "Player";
static char client_pipe_name[PIPE_NAME_LENGTH];
static int client_pipe_fd = -1;
static volatile int active = 1;

void cleanup(void) {
    if (client_id > 0) {
        char disconnect_msg[BUFFER_SIZE];
        snprintf(disconnect_msg, BUFFER_SIZE, "DISCONNECT|%d", client_id);
        
        int server_fd = open(PIPE_SERVER_NAME, O_WRONLY | O_NONBLOCK);
        if (server_fd != -1) {
            write(server_fd, disconnect_msg, strlen(disconnect_msg));
            write(server_fd, "\n", 1);
            close(server_fd);
        }
        
        if (client_pipe_fd != -1) {
            close(client_pipe_fd);
        }
        
        unlink(client_pipe_name);
    }
    
    printf("\nЗавершение работы клиента...\n");
}

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        active = 0;
    }
}

int connect_to_server(int id, const char *name) {
    client_id = id;
    
    if (name != NULL && strlen(name) > 0) {
        strncpy(client_name, name, MAX_NAME_LEN - 1);
        client_name[MAX_NAME_LEN - 1] = '\0';
    } else {
        snprintf(client_name, MAX_NAME_LEN, "Player%d", id);
    }
    
    snprintf(client_pipe_name, PIPE_NAME_LENGTH, PIPE_CLIENT_TEMPLATE, id);
    unlink(client_pipe_name);
    
    if (mkfifo(client_pipe_name, 0666) == -1) {
        perror("mkfifo client pipe");
        return -1;
    }
    
    printf("Подключение к серверу...\n");
    printf("Имя игрока: %s (ID: %d)\n", client_name, client_id);
    
    char connect_msg[BUFFER_SIZE];
    snprintf(connect_msg, BUFFER_SIZE, "CONNECT|%d|%s", id, client_name);
    
    int server_fd = open(PIPE_SERVER_NAME, O_WRONLY);
    if (server_fd == -1) {
        perror("open server pipe");
        unlink(client_pipe_name);
        return -1;
    }
    
    write(server_fd, connect_msg, strlen(connect_msg));
    write(server_fd, "\n", 1);
    close(server_fd);
    
    printf("Ожидание подтверждения от сервера...\n");
    client_pipe_fd = open(client_pipe_name, O_RDONLY | O_NONBLOCK);
    if (client_pipe_fd == -1) {
        perror("open client pipe");
        unlink(client_pipe_name);
        return -1;
    }
    
    printf("Подключено к серверу!\n");
    
    return 0;
}

int send_to_server(const char *message) {
    int server_fd = open(PIPE_SERVER_NAME, O_WRONLY);
    if (server_fd == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "Ошибка: сервер недоступен\n");
            active = 0;
        } else {
            fprintf(stderr, "Ошибка открытия серверного pipe: %s\n", strerror(errno));
        }
        return -1;
    }
    
    ssize_t bytes = write(server_fd, message, strlen(message));
    if (bytes > 0) {
        write(server_fd, "\n", 1);
    }
    close(server_fd);
    
    if (bytes < 0) {
        fprintf(stderr, "Ошибка записи в серверный pipe: %s\n", strerror(errno));
        return -1;
    }
    
    return 0;
}

void handle_server_message(const char *buffer) {
    char msg_copy[BUFFER_SIZE];
    strncpy(msg_copy, buffer, BUFFER_SIZE - 1);
    msg_copy[BUFFER_SIZE - 1] = '\0';
    
    char *newline = strchr(msg_copy, '\n');
    if (newline) *newline = '\0';
    
    if (strlen(msg_copy) == 0) {
        return;
    }
    
    char *token = strtok(msg_copy, "|");
    if (token == NULL) {
        return;
    }
    
    if (strcmp(token, "CONNECT") == 0) {
        token = strtok(NULL, "|");
        if (token == NULL) return;
        
        token = strtok(NULL, "|");
        if (token == NULL) return;
        
        printf("> %s\n", token);
        
    } else if (strcmp(token, "GAME_STATE") == 0) {
        token = strtok(NULL, "|");
        if (token == NULL) return;
        
        printf("> %s\n", token);
        
    } else if (strcmp(token, "RESULT") == 0) {
        token = strtok(NULL, "|");
        if (token == NULL) return;
        int player_id = atoi(token);
        
        token = strtok(NULL, "|");
        if (token == NULL) return;
        int bulls = atoi(token);
        
        token = strtok(NULL, "|");
        if (token == NULL) return;
        int cows = atoi(token);
        
        token = strtok(NULL, "|");
        if (token == NULL) return;
        const char *message = token;
        
        if (player_id == client_id) {
            int is_error = (strstr(message, "Неверный") != NULL || 
                           strstr(message, "формат") != NULL || 
                           strstr(message, "букв") != NULL || 
                           strstr(message, "Введите") != NULL);
            
            if (is_error) {
                printf("> %s\n", message);
            } else {
                printf("> Ваш результат: Быки: %d, Коровы: %d\n", bulls, cows);
            }
        } else {
            printf("> %s\n", message);
        }
        
    } else if (strcmp(token, "WIN") == 0) {
        token = strtok(NULL, "|");
        if (token == NULL) return;
        int winner_id = atoi(token);
        
        token = strtok(NULL, "|");
        if (token == NULL) return;
        const char *secret_word = token;
        
        token = strtok(NULL, "|");
        if (token == NULL) return;
        
        printf("\n=== %s ===\n", token);
        if (winner_id == client_id) {
            printf("Поздравляем! Вы угадали слово: %s\n", secret_word);
        } else {
            printf("Загаданное слово было: %s\n", secret_word);
        }
        printf("========================\n\n");
        
        active = 0;
        
    } else if (strcmp(token, "DISCONNECT") == 0) {
        token = strtok(NULL, "|");
        if (token == NULL) return;
        
        token = strtok(NULL, "|");
        if (token == NULL) return;
        
        printf("> %s\n", token);
    }
}

void handle_user_input(const char *input) {
    if (strlen(input) == 0) {
        return;
    }
    
    if (strcmp(input, "/quit") == 0 || strcmp(input, "/exit") == 0) {
        printf("Выход из игры...\n");
        active = 0;
        return;
    }
    
    if (strcmp(input, "/help") == 0) {
        printf("\n=== Команды ===\n");
        printf("/quit или /exit - выход из игры\n");
        printf("/help - показать эту справку\n");
        printf("===============\n\n");
        return;
    }
    
    char guess_msg[BUFFER_SIZE];
    snprintf(guess_msg, BUFFER_SIZE, "GUESS|%d|%s", client_id, input);
    send_to_server(guess_msg);
}

void client_loop(void) {
    fd_set read_fds;
    struct timeval timeout;
    char buffer[BUFFER_SIZE];
    
    printf("\n=== Игра 'Быки и коровы' ===\n");
    printf("Введите слово для угадывания или команду (/help для справки)\n\n");
    
    while (active) {
        FD_ZERO(&read_fds);
        
        if (client_pipe_fd >= 0) {
            FD_SET(client_pipe_fd, &read_fds);
        }
        FD_SET(STDIN_FILENO, &read_fds);
        
        int max_fd = STDIN_FILENO;
        if (client_pipe_fd >= 0 && client_pipe_fd > max_fd) {
            max_fd = client_pipe_fd;
        }
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            if (errno == EBADF) {
                if (client_pipe_fd >= 0) {
                    close(client_pipe_fd);
                }
                client_pipe_fd = open(client_pipe_name, O_RDONLY | O_NONBLOCK);
                if (client_pipe_fd == -1) {
                    printf("Соединение с сервером потеряно\n");
                    active = 0;
                    break;
                }
                continue;
            }
            perror("select");
            break;
        }
        
        if (FD_ISSET(client_pipe_fd, &read_fds)) {
            ssize_t bytes = read(client_pipe_fd, buffer, BUFFER_SIZE - 1);
            
            if (bytes > 0) {
                buffer[bytes] = '\0';
                
                char *line = buffer;
                char *line_end;
                while ((line_end = strchr(line, '\n')) != NULL) {
                    *line_end = '\0';
                    
                    if (strlen(line) > 0) {
                        handle_server_message(line);
                    }
                    
                    line = line_end + 1;
                }
                
                if (strlen(line) > 0) {
                    handle_server_message(line);
                }
            } else if (bytes == 0) {
                static int reopen_attempts = 0;
                
                close(client_pipe_fd);
                
                struct timespec delay = {0, 10000000};
                nanosleep(&delay, NULL);
                
                client_pipe_fd = open(client_pipe_name, O_RDONLY | O_NONBLOCK);
                if (client_pipe_fd == -1) {
                    reopen_attempts++;
                    if (reopen_attempts > 5) {
                        printf("Соединение с сервером потеряно\n");
                        active = 0;
                        break;
                    }
                } else {
                    reopen_attempts = 0;
                }
            } else if (bytes < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    printf("Ошибка чтения из pipe: %s\n", strerror(errno));
                }
            }
        }
        
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                buffer[strcspn(buffer, "\n")] = '\0';
                handle_user_input(buffer);
            } else {
                printf("\nВыход...\n");
                active = 0;
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Установка обработчиков сигналов
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    atexit(cleanup);
    
    int id = getpid();
    if (argc > 1) {
        id = atoi(argv[1]);
    }
    
    const char *name = NULL;
    if (argc > 2) {
        name = argv[2];
    } else {
        printf("Введите ваше имя: ");
        fflush(stdout);
        char name_input[MAX_NAME_LEN];
        if (fgets(name_input, MAX_NAME_LEN, stdin) != NULL) {
            name_input[strcspn(name_input, "\n")] = '\0';
            if (strlen(name_input) > 0) {
                name = name_input;
            }
        }
    }
    
    if (connect_to_server(id, name) != 0) {
        fprintf(stderr, "Ошибка подключения к серверу\n");
        return 1;
    }
    
    client_loop();
    
    return 0;
}

