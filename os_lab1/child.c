#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    const char *filename = argv[1];
    FILE *out = fopen(filename, "w");
    if (!out) {
        perror("fopen output file");
        return 1;
    }

    // Читаем строки из stdin
    char line[512];
    while (fgets(line, sizeof(line), stdin)) {
        float numbers[128];
        int count = 0;
        char *ptr = line;
        char *endptr;

        while (*ptr != '\0') {
            // Пропуск пробелов
            while (*ptr == ' ' || *ptr == '\t') {
                ptr++;
            }
            if (*ptr == '\0' || *ptr == '\n')
                break;

            // Преобразуем строку в число
            float value = strtof(ptr, &endptr);
            if (ptr == endptr) {
                break;
            }
            if (count < (int)(sizeof(numbers) / sizeof(numbers[0]))) {
                numbers[count++] = value;
            }
            ptr = endptr;
        }

        if (count <= 0) {
            continue;
        }

        // Вычисление результата деления
        float result = numbers[0];
        for (int i = 1; i < count; ++i) {
            // Проверка деления на ноль
            if (numbers[i] == 0.0f) {
                const char *msg = "Деление на 0. Завершение.\n";
                fprintf(out, "Числа на входе: %sОшибка: деление на ноль\n\n", line);
                fflush(out);
                fputs(msg, stdout);
                fflush(stdout);
                fclose(out);
                return 0;
            }
            result /= numbers[i];
        }

        // Запись результата в файл
        fprintf(out, "Полученные числа: %sРезультат: %f\n\n", line, result);
        fflush(out);
    }

    fclose(out);
    return 0;
}
