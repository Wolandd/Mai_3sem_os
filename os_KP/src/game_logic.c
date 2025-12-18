#include "game_logic.h"
#include <time.h>
#include <ctype.h>

#define MAX_WORDS 100
#define WORDS_FILE "words.txt"

static char **word_list = NULL;
static int word_list_size = 0;
static int word_list_capacity = 0;

int load_word_dictionary(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        LOG_ERROR("Не удалось открыть файл словаря: %s", filename);
        return -1;
    }
    
    if (word_list != NULL) {
        for (int i = 0; i < word_list_size; i++) {
            free(word_list[i]);
        }
        free(word_list);
        word_list = NULL;
        word_list_size = 0;
        word_list_capacity = 0;
    }
    
    word_list_capacity = MAX_WORDS;
    word_list = (char**)malloc(word_list_capacity * sizeof(char*));
    if (word_list == NULL) {
        LOG_ERROR("Ошибка выделения памяти для словаря");
        fclose(fp);
        return -1;
    }
    
    char line[BUFFER_SIZE];
    word_list_size = 0;
    
    while (fgets(line, sizeof(line), fp) != NULL && word_list_size < MAX_WORDS) {
        line[strcspn(line, "\n\r")] = '\0';
        
        if (strlen(line) == 0) {
            continue;
        }
        
        int valid = 1;
        for (int i = 0; line[i] != '\0'; i++) {
            if (!isalpha((unsigned char)line[i])) {
                valid = 0;
                break;
            }
        }
        
        if (!valid) {
            LOG_DEBUG("Пропущено недопустимое слово: %s", line);
            continue;
        }
        
        word_list[word_list_size] = (char*)malloc(strlen(line) + 1);
        if (word_list[word_list_size] == NULL) {
            LOG_ERROR("Ошибка выделения памяти для слова");
            break;
        }
        
        strcpy(word_list[word_list_size], line);
        word_list_size++;
    }
    
    fclose(fp);
    
    if (word_list_size == 0) {
        LOG_ERROR("Словарь пуст или не содержит валидных слов");
        free(word_list);
        word_list = NULL;
        return -1;
    }
    
    LOG_INFO("Загружено %d слов из файла %s", word_list_size, filename);
    return word_list_size;
}

int generate_secret_word(char *secret_word) {
    static int initialized = 0;
    
    if (!initialized) {
        srand(time(NULL));
        
        if (load_word_dictionary(WORDS_FILE) < 0) {
            LOG_ERROR("Не удалось загрузить словарь, используем встроенный");
            
            static const char *fallback_words[] = {
                "apple", "house", "computer", "book", "window",
                "table", "chair", "mouse", "keyboard", "screen"
            };
            int fallback_size = 10;
            
            word_list_capacity = fallback_size;
            word_list = (char**)malloc(word_list_capacity * sizeof(char*));
            if (word_list == NULL) {
                return -1;
            }
            
            for (int i = 0; i < fallback_size; i++) {
                word_list[i] = (char*)malloc(strlen(fallback_words[i]) + 1);
                if (word_list[i] != NULL) {
                    strcpy(word_list[i], fallback_words[i]);
                }
            }
            word_list_size = fallback_size;
        }
        
        initialized = 1;
    }
    
    if (word_list_size == 0) {
        LOG_ERROR("Словарь пуст");
        return -1;
    }
    
    int index = rand() % word_list_size;
    const char *selected_word = word_list[index];
    
    strncpy(secret_word, selected_word, MAX_WORD_LEN - 1);
    secret_word[MAX_WORD_LEN - 1] = '\0';
    
    LOG_DEBUG("Сгенерировано слово: %s (индекс: %d)", secret_word, index);
    
    return 0;
}

void check_guess(const char *secret, const char *guess, int *bulls, int *cows) {
    *bulls = 0;
    *cows = 0;
    
    int secret_len = strlen(secret);
    int guess_len = strlen(guess);
    
    if (secret_len != guess_len) {
        return;
    }
    
    int secret_used[secret_len];
    int guess_used[guess_len];
    
    for (int i = 0; i < secret_len; i++) {
        secret_used[i] = 0;
        guess_used[i] = 0;
    }
    
    for (int i = 0; i < secret_len; i++) {
        if (secret[i] == guess[i]) {
            (*bulls)++;
            secret_used[i] = 1;
            guess_used[i] = 1;
        }
    }
    
    for (int i = 0; i < guess_len; i++) {
        if (guess_used[i]) {
            continue;
        }
        
        for (int j = 0; j < secret_len; j++) {
            if (secret_used[j]) {
                continue;
            }
            
            if (secret[j] == guess[i]) {
                (*cows)++;
                secret_used[j] = 1;
                guess_used[i] = 1;
                break;
            }
        }
    }
}

int validate_guess(const char *guess, const char *secret) {
    int guess_len = strlen(guess);
    int secret_len = strlen(secret);
    
    if (guess_len != secret_len) {
        LOG_DEBUG("Неверная длина слова: %d (ожидается %d)", guess_len, secret_len);
        return 0;
    }
    
    for (int i = 0; i < guess_len; i++) {
        if (!isalpha((unsigned char)guess[i])) {
            LOG_DEBUG("Слово содержит недопустимые символы: %s", guess);
            return 0;
        }
    }
    
    return 1;
}

int is_word_guessed(int bulls, int word_length) {
    return (bulls == word_length);
}

