
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

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

static int utoa(unsigned int x, char *out) {
    char tmp[20];
    int n = 0;
    if (x == 0) { out[0] = '0'; return 1; }
    while (x > 0 && n < (int)sizeof(tmp)) {
        tmp[n++] = (char)('0' + (x % 10));
        x /= 10;
    }
    for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    return n;
}

static int ftoa3(float v, char *out) {
    int pos = 0;
    if (v < 0) { out[pos++] = '-'; v = -v; }

    unsigned int ip = (unsigned int)v;
    float frac = v - (float)ip;
    unsigned int fp = (unsigned int)(frac * 1000.0f + 0.5f);
    if (fp >= 1000) { fp -= 1000; ip += 1; }

    pos += utoa(ip, out + pos);
    out[pos++] = '.';
    out[pos++] = (char)('0' + (fp / 100) % 10);
    out[pos++] = (char)('0' + (fp / 10) % 10);
    out[pos++] = (char)('0' + (fp % 10));
    return pos;
}

int main(int argc, char **argv) {
    if (argc < 2) die("no output file");
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) die("open failed");

    char line[1024];

    while (1) {
        int n = read_line(0, line, (int)sizeof(line));
        if (n == 0) break;
        if (n < 0) die("read error");

        char *p = line;
        char *end;

        float a = strtof(p, &end);
        if (end == p) {
            char msg[] = "bad line\n";
            write(fd, msg, sizeof(msg)-1);
            write(1, "O", 1);
            continue;
        }
        p = end;

        float res = a;
        int have_div = 0;

        while (1) {
            float b = strtof(p, &end);
            if (end == p) break;
            have_div = 1;

            if (b == 0.0f) {
                char msg[] = "division by zero\n";
                write(fd, msg, sizeof(msg)-1);
                write(1, "Z", 1);
                close(fd);
                return 0;
            }
            res = res / b;
            p = end;
        }

        if (!have_div) {
            char msg[] = "need at least 2 numbers\n";
            write(fd, msg, sizeof(msg)-1);
            write(1, "O", 1);
            continue;
        }

        char out[64];
        int k = 0;
        out[k++] = 'r'; out[k++] = 'e'; out[k++] = 's'; out[k++] = '=';
        k += ftoa3(res, out + k);
        out[k++] = '\n';
        write(fd, out, (size_t)k);

        write(1, "O", 1);
    }

    close(fd);
    return 0;
}
