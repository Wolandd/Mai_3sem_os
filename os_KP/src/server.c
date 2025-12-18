#include "shared.h"
#include "game_logic.h"
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>

static Game game;
static int server_fd = -1;
static int required_players = MAX_PLAYERS;
static volatile int running = 1;

void cleanup_client_pipes(void);
void handle_client_connect(int player_id, const char *player_name);
void handle_client_guess(int player_id, const char *guess_word);
void handle_client_disconnect(int player_id);
void broadcast_message(const char *message);

void cleanup(void) {
    LOG_INFO("Завершение работы сервера...");
    printf("\nЗавершение работы сервера...\n");
    
    if (server_fd != -1) {
        close(server_fd);
        LOG_DEBUG("Закрыт файловый дескриптор сервера");
    }
    
    unlink(PIPE_SERVER_NAME);
    cleanup_client_pipes();
    
    LOG_INFO("Ресурсы освобождены");
    printf("Ресурсы освобождены\n");
}

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    } else if (sig == SIGPIPE) {
        return;
    }
}

int setup_server(void) {
    LOG_INFO("Инициализация сервера...");
    
    unlink(PIPE_SERVER_NAME);
    LOG_DEBUG("Удален старый pipe: %s", PIPE_SERVER_NAME);
    
    if (mkfifo(PIPE_SERVER_NAME, 0666) == -1) {
        LOG_ERROR("Ошибка создания pipe: %s", strerror(errno));
        perror("mkfifo");
        return -1;
    }
    LOG_DEBUG("Создан pipe: %s", PIPE_SERVER_NAME);
    
    server_fd = open(PIPE_SERVER_NAME, O_RDONLY | O_NONBLOCK);
    if (server_fd == -1) {
        LOG_ERROR("Ошибка открытия pipe: %s", strerror(errno));
        perror("open server pipe");
        unlink(PIPE_SERVER_NAME);
        return -1;
    }
    LOG_DEBUG("Открыт pipe для чтения, fd=%d", server_fd);
    
    memset(&game, 0, sizeof(Game));
    game.player_count = 0;
    game.game_active = 0;
    
    if (generate_secret_word(game.secret_word) != 0) {
        LOG_ERROR("Ошибка генерации секретного слова");
        return -1;
    }
    
    LOG_INFO("Сервер инициализирован, секретное слово: %s", game.secret_word);
    printf("Сервер инициализирован\n");
    
    return 0;
}

int find_player_index(int player_id) {
    for (int i = 0; i < game.player_count; i++) {
        if (game.players[i].id == player_id && game.players[i].active) {
            return i;
        }
    }
    return -1;
}

int validate_message(const char *buffer) {
    if (buffer == NULL) {
        return 0;
    }
    
    const char *start = buffer;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start++;
    }
    
    if (*start == '\0') {
        LOG_DEBUG("Получено пустое сообщение");
        return 0;
    }
    
    size_t len = strlen(start);
    if (len >= BUFFER_SIZE) {
        LOG_ERROR("Сообщение слишком длинное: %zu байт", len);
        return 0;
    }
    
    if (strchr(start, '|') == NULL) {
        LOG_DEBUG("Сообщение не содержит разделитель: %s", start);
        return 0;
    }
    
    return 1;
}

int send_text_to_player(int player_index, const char *message) {
    if (player_index < 0 || player_index >= game.player_count) {
        LOG_ERROR("Неверный индекс игрока: %d", player_index);
        return -1;
    }
    
    Player *player = &game.players[player_index];
    if (!player->active) {
        LOG_DEBUG("Игрок %s неактивен", player->name);
        return -1;
    }
    
    int client_fd = open(player->pipe_name, O_WRONLY);
    if (client_fd == -1) {
        if (errno == ENOENT) {
            LOG_ERROR("Pipe игрока %s не существует: %s", player->name, strerror(errno));
            player->active = 0;
            printf("Игрок %s отключился\n", player->name);
        } else if (errno == EPIPE) {
            LOG_ERROR("Pipe игрока %s закрыт: %s", player->name, strerror(errno));
            player->active = 0;
            printf("Игрок %s отключился\n", player->name);
        } else {
            LOG_ERROR("Ошибка открытия pipe игрока %s: %s", player->name, strerror(errno));
        }
        return -1;
    }
    
    ssize_t bytes = write(client_fd, message, strlen(message));
    if (bytes > 0) {
        write(client_fd, "\n", 1);
    }
    close(client_fd);
    
    if (bytes == -1) {
        if (errno == EPIPE) {
            LOG_ERROR("Разрыв соединения с игроком %s", player->name);
            player->active = 0;
            printf("Игрок %s отключился\n", player->name);
        } else {
            LOG_ERROR("Ошибка записи в pipe игрока %s: %s", player->name, strerror(errno));
        }
        return -1;
    }
    
    LOG_DEBUG("Отправлено сообщение игроку %s: %s", player->name, message);
    return 0;
}

void broadcast_message(const char *message) {
    for (int i = 0; i < game.player_count; i++) {
        if (game.players[i].active) {
            struct stat st;
            if (stat(game.players[i].pipe_name, &st) == 0) {
                send_text_to_player(i, message);
            } else {
                LOG_DEBUG("Pipe игрока %s не существует, помечаем как неактивного", game.players[i].name);
                game.players[i].active = 0;
            }
        }
    }
}

void cleanup_client_pipes(void) {
    for (int i = 0; i < game.player_count; i++) {
        if (!game.players[i].active && strlen(game.players[i].pipe_name) > 0) {
            unlink(game.players[i].pipe_name);
            printf("Очищен pipe игрока %s\n", game.players[i].name);
        }
    }
}

void handle_client_connect(int player_id, const char *player_name) {
    LOG_INFO("Обработка подключения игрока: ID=%d, Имя=%s", player_id, player_name);
    
    if (find_player_index(player_id) != -1) {
        LOG_ERROR("Игрок с ID %d уже подключен", player_id);
        printf("Игрок с ID %d уже подключен\n", player_id);
        return;
    }
    
    if (game.player_count >= required_players) {
        LOG_ERROR("Достигнуто максимальное количество игроков: %d", required_players);
        printf("Достигнуто максимальное количество игроков\n");
        return;
    }
    
    char client_pipe_name[PIPE_NAME_LENGTH];
    snprintf(client_pipe_name, PIPE_NAME_LENGTH, PIPE_CLIENT_TEMPLATE, player_id);
    
    struct stat st;
    if (stat(client_pipe_name, &st) != 0) {
        LOG_ERROR("Pipe клиента не существует: %s", client_pipe_name);
        printf("Ошибка: pipe клиента не найден\n");
        return;
    }
    LOG_DEBUG("Pipe клиента найден: %s", client_pipe_name);
    
    Player *player = &game.players[game.player_count];
    player->id = player_id;
    strncpy(player->name, player_name, MAX_NAME_LEN - 1);
    player->name[MAX_NAME_LEN - 1] = '\0';
    strncpy(player->pipe_name, client_pipe_name, PIPE_NAME_LENGTH - 1);
    player->pipe_name[PIPE_NAME_LENGTH - 1] = '\0';
    player->active = 1;
    
    game.player_count++;
    
    printf("Игрок %s (ID: %d) присоединился. Игроков: %d/%d\n", 
           player_name, player_id, game.player_count, required_players);
    
    char welcome_msg[BUFFER_SIZE];
    snprintf(welcome_msg, BUFFER_SIZE, 
            "CONNECT|%d|Вы подключены! Ожидаем других игроков (%d/%d)", 
            player_id, game.player_count, required_players);
    LOG_DEBUG("Отправка приветственного сообщения игроку %s", player_name);
    if (send_text_to_player(game.player_count - 1, welcome_msg) != 0) {
        LOG_ERROR("Не удалось отправить приветственное сообщение игроку %s", player_name);
    } else {
        LOG_DEBUG("Приветственное сообщение отправлено успешно");
    }
    
    if (game.player_count >= required_players) {
        game.game_active = 1;
        
        char game_start_msg[BUFFER_SIZE];
        snprintf(game_start_msg, BUFFER_SIZE, 
                "GAME_STATE|Все игроки подключены! Игра началась. Слово состоит из %d букв.",
                (int)strlen(game.secret_word));
        
        broadcast_message(game_start_msg);
        printf("Игра началась!\n");
    }
}

void handle_client_guess(int player_id, const char *guess_word) {
    LOG_DEBUG("Обработка предположения: ID=%d, Слово=%s", player_id, guess_word);
    
    if (!game.game_active) {
        LOG_DEBUG("Игра не активна, игнорируем предположение");
        return;
    }
    
    int player_index = find_player_index(player_id);
    if (player_index == -1) {
        LOG_ERROR("Игрок с ID %d не найден", player_id);
        return;
    }
    
    Player *player = &game.players[player_index];
    
    if (!validate_guess(guess_word, game.secret_word)) {
        LOG_DEBUG("Неверное предположение от игрока %s: %s", player->name, guess_word);
        char error_msg[BUFFER_SIZE];
        snprintf(error_msg, BUFFER_SIZE, 
                "RESULT|%d|0|0|Неверный формат! Введите слово из %d букв (только буквы).",
                player_id, (int)strlen(game.secret_word));
        send_text_to_player(player_index, error_msg);
        return;
    }
    
    int bulls, cows;
    check_guess(game.secret_word, guess_word, &bulls, &cows);
    
    if (is_word_guessed(bulls, strlen(game.secret_word))) {
        game.game_active = 0;
        
        char win_msg[BUFFER_SIZE];
        snprintf(win_msg, BUFFER_SIZE, 
                "WIN|%d|%s|ПОБЕДА! Игрок %s угадал слово '%s'!", 
                player_id, game.secret_word, player->name, game.secret_word);
        
        broadcast_message(win_msg);
        printf("Игрок %s выиграл! Слово: %s\n", player->name, game.secret_word);
        
        struct timespec delay = {1, 0};
        nanosleep(&delay, NULL);
        
        int active_count = 0;
        for (int i = 0; i < game.player_count; i++) {
            if (game.players[i].active) {
                active_count++;
            }
        }
        
        if (active_count == 0) {
            printf("Все игроки отключились после победы. Завершение работы сервера...\n");
            running = 0;
        }
    } else {
        char result_msg[BUFFER_SIZE];
        snprintf(result_msg, BUFFER_SIZE, 
                "RESULT|%d|%d|%d|Игрок %s: Быки: %d, Коровы: %d",
                player_id, bulls, cows, player->name, bulls, cows);
        
        broadcast_message(result_msg);
        printf("Игрок %s: %s -> Быки: %d, Коровы: %d\n", 
               player->name, guess_word, bulls, cows);
    }
}

void handle_client_disconnect(int player_id) {
    LOG_INFO("Обработка отключения игрока: ID=%d", player_id);
    
    int player_index = find_player_index(player_id);
    if (player_index == -1) {
        LOG_DEBUG("Игрок с ID %d не найден при отключении (возможно, уже отключен)", player_id);
        return;
    }
    
    Player *player = &game.players[player_index];
    
    player->active = 0;
    
    if (unlink(player->pipe_name) == 0) {
        LOG_DEBUG("Удален pipe игрока: %s", player->pipe_name);
    } else {
        LOG_DEBUG("Pipe игрока %s уже удален или не существует", player->pipe_name);
    }
    
    printf("Игрок %s (ID: %d) отключился\n", player->name, player_id);
    
    if (game.game_active) {
        char disconnect_msg[BUFFER_SIZE];
        snprintf(disconnect_msg, BUFFER_SIZE, 
                "DISCONNECT|%d|Игрок %s отключился", player_id, player->name);
        broadcast_message(disconnect_msg);
    }
    
    int active_count = 0;
    for (int i = 0; i < game.player_count; i++) {
        if (game.players[i].active) {
            active_count++;
        }
    }
    
    if (active_count == 0) {
        if (game.game_active) {
            game.game_active = 0;
            char game_over_msg[BUFFER_SIZE];
            snprintf(game_over_msg, BUFFER_SIZE, 
                    "GAME_STATE|Игра остановлена: все игроки отключились");
            broadcast_message(game_over_msg);
            printf("Игра остановлена: все игроки отключились\n");
        } else {
            printf("Все игроки отключились после завершения игры.\n");
        }
        
        printf("Завершение работы сервера...\n");
        running = 0;
    } else if (active_count > 0 && game.game_active) {
        printf("Игра продолжается. Активных игроков: %d\n", active_count);
    }
}

void parse_message(const char *buffer) {
    if (buffer == NULL) {
        return;
    }
    
    char msg_copy[BUFFER_SIZE];
    strncpy(msg_copy, buffer, BUFFER_SIZE - 1);
    msg_copy[BUFFER_SIZE - 1] = '\0';
    
    char *newline = strchr(msg_copy, '\n');
    if (newline) *newline = '\0';
    
    // Удаляем пробелы в начале и конце
    char *start = msg_copy;
    while (*start == ' ' || *start == '\t' || *start == '\r') {
        start++;
    }
    
    size_t len = strlen(start);
    if (len > 0) {
        char *end = start + len - 1;
        while (end >= start && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
            *end = '\0';
            if (end == start) break;
            end--;
        }
    }
    
    if (strlen(start) == 0) {
        LOG_DEBUG("Получено пустое сообщение после обработки");
        return;
    }
    
    if (!validate_message(start)) {
        LOG_DEBUG("Получено неверное сообщение: %s", start);
        return;
    }
    
    LOG_DEBUG("Парсинг сообщения: %s", start);
    
    char *token = strtok(start, "|");
    if (token == NULL) {
        LOG_ERROR("Не удалось извлечь тип сообщения");
        return;
    }
    
    if (strcmp(token, "CONNECT") == 0) {
        token = strtok(NULL, "|");
        if (token == NULL) {
            LOG_ERROR("Неверный формат CONNECT: отсутствует player_id");
            return;
        }
        int player_id = atoi(token);
        if (player_id <= 0) {
            LOG_ERROR("Неверный player_id: %s", token);
            return;
        }
        
        token = strtok(NULL, "|");
        if (token == NULL) {
            LOG_ERROR("Неверный формат CONNECT: отсутствует player_name");
            return;
        }
        
        if (strlen(token) == 0 || strlen(token) >= MAX_NAME_LEN) {
            LOG_ERROR("Неверная длина имени игрока: %zu", strlen(token));
            return;
        }
        
        handle_client_connect(player_id, token);
        
    } else if (strcmp(token, "GUESS") == 0) {
        if (!game.game_active) {
            LOG_DEBUG("Игра завершена, игнорируем предположение");
            return;
        }
        
        token = strtok(NULL, "|");
        if (token == NULL) {
            LOG_ERROR("Неверный формат GUESS: отсутствует player_id");
            return;
        }
        int player_id = atoi(token);
        if (player_id <= 0) {
            LOG_ERROR("Неверный player_id: %s", token);
            return;
        }
        
        token = strtok(NULL, "|");
        if (token == NULL) {
            LOG_ERROR("Неверный формат GUESS: отсутствует слово");
            return;
        }
        
        if (strlen(token) == 0 || strlen(token) >= MAX_WORD_LEN) {
            LOG_ERROR("Неверная длина слова: %zu", strlen(token));
            return;
        }
        
        handle_client_guess(player_id, token);
        
    } else if (strcmp(token, "DISCONNECT") == 0) {
        token = strtok(NULL, "|");
        if (token == NULL) {
            LOG_ERROR("Неверный формат DISCONNECT: отсутствует player_id");
            return;
        }
        int player_id = atoi(token);
        if (player_id <= 0) {
            LOG_ERROR("Неверный player_id: %s", token);
            return;
        }
        
        handle_client_disconnect(player_id);
    } else {
        LOG_ERROR("Неизвестный тип сообщения: %s", token);
    }
}

void game_loop(void) {
    fd_set read_fds;
    struct timeval timeout;
    char buffer[BUFFER_SIZE];
    
    printf("Ожидание подключения игроков (%d/%d)...\n", 
           game.player_count, required_players);
    
    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }
        
        if (FD_ISSET(server_fd, &read_fds)) {
            ssize_t bytes = read(server_fd, buffer, BUFFER_SIZE - 1);
            
            if (bytes > 0) {
                buffer[bytes] = '\0';
                LOG_DEBUG("Получено сообщение от клиента (%zd байт): %s", bytes, buffer);
                
                char *line = buffer;
                char *line_end;
                while ((line_end = strchr(line, '\n')) != NULL) {
                    *line_end = '\0';
                    
                    if (strlen(line) > 0) {
                        parse_message(line);
                    }
                    
                    line = line_end + 1;
                }
                
                if (strlen(line) > 0) {
                    parse_message(line);
                }
            } else if (bytes == 0) {
                LOG_DEBUG("Получен EOF от клиента, переоткрываем серверный pipe");
                close(server_fd);
                
                struct timespec delay = {0, 10000000};
                nanosleep(&delay, NULL);
                
                server_fd = open(PIPE_SERVER_NAME, O_RDONLY | O_NONBLOCK);
                if (server_fd == -1) {
                    LOG_ERROR("Ошибка переоткрытия серверного pipe: %s", strerror(errno));
                    unlink(PIPE_SERVER_NAME);
                    if (mkfifo(PIPE_SERVER_NAME, 0666) == -1) {
                        LOG_ERROR("Ошибка пересоздания pipe: %s", strerror(errno));
                        break;
                    }
                    server_fd = open(PIPE_SERVER_NAME, O_RDONLY | O_NONBLOCK);
                    if (server_fd == -1) {
                        LOG_ERROR("Критическая ошибка: не удалось открыть серверный pipe");
                        break;
                    }
                }
                LOG_DEBUG("Серверный pipe переоткрыт, fd=%d", server_fd);
            } else if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG_ERROR("Ошибка чтения из серверного pipe: %s", strerror(errno));
            }
        }
        
        static int cleanup_counter = 0;
        if (++cleanup_counter >= 100) {
            cleanup_client_pipes();
            cleanup_counter = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        required_players = atoi(argv[1]);
        if (required_players < 1 || required_players > MAX_PLAYERS) {
            fprintf(stderr, "Количество игроков должно быть от 1 до %d\n", MAX_PLAYERS);
            return 1;
        }
    }
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);
    
    if (setup_server() != 0) {
        fprintf(stderr, "Ошибка инициализации сервера\n");
        return 1;
    }
    
    atexit(cleanup);
    
    printf("Сервер запущен. Ожидание %d игроков...\n", required_players);
    printf("Используйте pipe: %s\n", PIPE_SERVER_NAME);
    
    game_loop();
    
    return 0;
}

