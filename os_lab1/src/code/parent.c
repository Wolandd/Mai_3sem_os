#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

int main(void) {
    char filename[256];
    printf("Введите имя файла для вывода результатов: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Ошибка чтения имени файла\n");
        return 1;
    }

    // Удаление лишнего символа в конце имени файла
    size_t len = strlen(filename);
    if (len > 0 && filename[len - 1] == '\n') {
        filename[len - 1] = '\0';
    }

    int pipe_to_child[2];   // Отправка данных дочернему процессу
    int pipe_from_child[2]; // Получение данных от дочернего процесса

    if (pipe(pipe_to_child) == -1) {
        perror("pipe");
        return 1;
    }
    if (pipe(pipe_from_child) == -1) {
        perror("pipe");
        return 1;
    }

    // Создание дочернего процесса
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        if (dup2(pipe_to_child[0], STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            _exit(1);
        }
        if (dup2(pipe_from_child[1], STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            _exit(1);
        }

        // Закрываем неиспользуемые дескрипторы
        close(pipe_to_child[0]);
        close(pipe_to_child[1]);
        close(pipe_from_child[0]);
        close(pipe_from_child[1]);

        // Запуск child
        execl("./child", "child", filename, (char *)NULL);
        perror("execl");
        _exit(1);
    } else {
        // Закрываем неиспользуемые концы pipe
        close(pipe_to_child[0]);
        close(pipe_from_child[1]);

        // Неблокирующий режима для pipe
        fcntl(pipe_from_child[0], F_SETFL, O_NONBLOCK);

        char line[512];
        printf("Введите строки вида: \"число число число\" (float). Пустая строка или EOF — завершение.\n");

        while (1) {
            printf("> ");
            if (!fgets(line, sizeof(line), stdin)) {
                break;
            }

            if (strcmp(line, "\n") == 0) {
                break;
            }

            // Отправка строки дочернему процессу
            if (write(pipe_to_child[1], line, strlen(line)) == -1) {
                perror("write to child");
                break;
            }
        }

        // Закрываем pipe и ожиданием завершение дочернего процесса
        close(pipe_to_child[1]);
        close(pipe_from_child[0]);

        int status;
        waitpid(pid, &status, 0);
        printf("Родительский процесс завершён.\n");
    }

    return 0;
}
