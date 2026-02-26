
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

static void die(const char *msg) {
    size_t n = 0;
    while (msg[n]) n++;
    write(2, msg, n);
    write(2, "\n", 1);
    _exit(1);
}

static int read_line(int fd, char *buf, int max) {
    int i = 0;
    char c;
    while (i < max - 1) {
        ssize_t r = read(fd, &c, 1);
        if (r == 0) break;
        if (r < 0) return -1;
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = 0;
    return i;
}

int main(void) {
    int to_child[2];
    int from_child[2];
    pid_t pid;
    char filename[256];
    char line[1024];

    write(1, "Output file name: ", 18);
    if (read_line(0, filename, (int)sizeof(filename)) <= 0) die("No file name");

    for (int i = 0; filename[i]; i++) {
        if (filename[i] == '\n') { filename[i] = 0; break; }
    }

    if (pipe(to_child) < 0) die("pipe failed");
    if (pipe(from_child) < 0) die("pipe failed");

    pid = fork();
    if (pid < 0) die("fork failed");

    if (pid == 0) {
        close(to_child[1]);
        close(from_child[0]);

        if (dup2(to_child[0], 0) < 0) die("dup2 failed");
        if (dup2(from_child[1], 1) < 0) die("dup2 failed");

        close(to_child[0]);
        close(from_child[1]);

        char *argv[] = { "./child", filename, 0 };
        execv("./child", argv);
        die("execv failed");
    }

    close(to_child[0]);
    close(from_child[1]);

    while (1) {
        int n = read_line(0, line, (int)sizeof(line));
        if (n == 0) break;
        if (n < 0) die("read error");

        if (n == 1 && line[0] == '\n') continue;

        if (write(to_child[1], line, (size_t)n) != n) die("write to child failed");

        char st;
        ssize_t r = read(from_child[0], &st, 1);
        if (r <= 0) break;
        if (st == 'Z') break;
    }

    close(to_child[1]);
    close(from_child[0]);

    waitpid(pid, 0, 0);
    return 0;
}
