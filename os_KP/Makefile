CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
DEBUG_FLAGS = -DDEBUG=1
LDFLAGS = 
INCLUDES = -Iinclude

# Директории
SRC_DIR = src
INCLUDE_DIR = include

# Исходные файлы
SERVER_SRC = $(SRC_DIR)/server.c $(SRC_DIR)/game_logic.c
CLIENT_SRC = $(SRC_DIR)/client.c

# Объектные файлы
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

# Исполняемые файлы
SERVER = server
CLIENT = client

.PHONY: all server client clean distclean debug

# Сборка сервера и клиента
all: $(SERVER) $(CLIENT)

# Компиляция сервера (линкуется с game_logic.c)
server: $(SERVER)

$(SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJ) $(LDFLAGS)
	@echo "Сервер собран: $(SERVER)"

# Компиляция клиента
client: $(CLIENT)

$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJ) $(LDFLAGS)
	@echo "Клиент собран: $(CLIENT)"

# Правило для компиляции объектных файлов
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Удаление скомпилированных файлов
clean:
	rm -f $(SERVER) $(CLIENT) $(SRC_DIR)/*.o
	@echo "Скомпилированные файлы удалены"

# Полная очистка (включая pipe'ы)
distclean: clean
	rm -f /tmp/bulls_cows_*
	rm -f server.log
	@echo "Полная очистка завершена (включая pipe'ы и логи)"

# Сборка с отладочными флагами
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all
	@echo "Сборка с отладкой завершена"
