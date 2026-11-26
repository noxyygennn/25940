
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>

#define STR_LEN 40
#define MAX_LEN 8192

#define CTRL_D 0x04
#define CTRL_W 0x17
#define CTRL_G 0x07
#define ESC 0x1B

char buffer[MAX_LEN + 1] = "";
int char_row[MAX_LEN];

int len = 0, rows = 1;

void bell() {
    unsigned char c = CTRL_G;
    write(STDOUT_FILENO, &c, 1);
}

void ignore_escape_sequence() {
    unsigned char c;

    if (read(STDIN_FILENO, &c, 1) != 1) return;
    if (c == '[') {
        while (read(STDIN_FILENO, &c, 1) == 1) {
            if (c >= 0x40 && c <= 0x7E) break;
        }
    }
    else if (c == 'O') read(STDIN_FILENO, &c, 1);
}

/* удаление диапазона символов */
void delete_range(int from, int to) {
    if (from >= to || from < 0 || to > len) return;
    memmove(buffer + from, buffer + to, len - to + 1);
    len -= to - from;
}

/* удаление последнего слова и следующих за ним пробелов */
void delete_word() {
    int start = len;
    while (start > 0 && buffer[start - 1] == ' ') { start--; }
    while (start > 0 && buffer[start - 1] != ' ' && buffer[start - 1] != '\n') { start--; }

    if (start == len) {
        bell();
        return;
    }
    delete_range(start, len);
}

/* Очистка текущей физической строки терминала */
void kill_physical_line() {
    int current_row = rows - 1;
    int start = len;

    while (start > 0 && buffer[start - 1] != '\n' && char_row[start - 1] == current_row) { start--; }
    delete_range(start, len);
}

/* перерисовка всего текста */
void redraw() {
    /* очищаем старое отображение */
    if (rows > 0) {
        char seq[32];
        if (rows > 1) {
            snprintf(seq, sizeof(seq), "\033[%dA", rows - 1);
            write(STDOUT_FILENO, seq, strlen(seq));
        }
        write(STDOUT_FILENO, "\r", 1);
        for (int i = 0; i < rows; i++) {
            write(STDOUT_FILENO, "\033[2K", 4);

            if (i + 1 < rows) write(STDOUT_FILENO, "\033[1B", 4);
        }

        if (rows > 1) {
            snprintf(seq, sizeof(seq), "\033[%dA", rows - 1);
            write(STDOUT_FILENO, seq, strlen(seq));
        }
        write(STDOUT_FILENO, "\r", 1);
    }

    int row = 0, col = 0;

    for (int i = 0; i < len;) {

        /* ENTER */
        if (buffer[i] == '\n') {
            char_row[i] = row;

            write(STDOUT_FILENO, "\r\n", 2);
            row++;
            col = 0;
            i++;
            continue;
        }

        /* SPACE */
        if (buffer[i] == ' ') {
            if (col == STR_LEN) {
                write(STDOUT_FILENO, "\r\n", 2);
                row++;
                col = 0;
            }

            char_row[i] = row;
            write(STDOUT_FILENO, " ", 1);
            col++;
            i++;
            continue;
        }

        /* определяем длину слова */
        int start = i;
        while (i < len && buffer[i] != ' ' && buffer[i] != '\n') { i++; }
        int word_len = i - start;

        /*  перенос целого слова */
        if (col > 0 && col + word_len > STR_LEN) {
            write(STDOUT_FILENO, "\r\n", 2);
            row++;
            col = 0;
        }

        /* Выводим слово */
        for (int j = start; j < i; j++) {
            if (col == STR_LEN) {
                write(STDOUT_FILENO, "\r\n", 2);
                row++;
                col = 0;
            }

            char_row[j] = row;
            write(STDOUT_FILENO, &buffer[j], 1);
            col++;
        }
    }

    rows = row + 1;
}

int main() {
    struct termios origg, coppy;

    if (tcgetattr(STDIN_FILENO, &origg) == -1) {
        perror("tcgetattr");
        return 1;
    }

    coppy = origg;
    coppy.c_lflag &= ~(ECHO | ICANON | ISIG);
    coppy.c_iflag &= ~(IXON | IXOFF);
    coppy.c_cc[VMIN] = 1;
    coppy.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &coppy) == -1) {
        perror("tcsetattr");
        return 1;
    }

    unsigned char c;

    redraw();

    while (read(STDIN_FILENO, &c, 1) == 1) {

        if (c == ESC) {
            ignore_escape_sequence();
            continue;
        }

        /* CTRL+D */
        if (c == CTRL_D) {
            if (len == 0 || buffer[len - 1] == '\n') break;

            bell();
            continue;
        }

        /* ENTER */
        if (c == '\n' || c == '\r') {
            if (len == MAX_LEN) {
                bell();
                continue;
            }

            buffer[len++] = '\n';
            buffer[len] = '\0';
        }

        /* ERASE / BACKSPACE */
        else if (c == origg.c_cc[VERASE]) {
            if (len == 0) {
                bell();
                continue;
            }
            delete_range(len - 1, len);
        }
        /* KILL */
        else if (c == origg.c_cc[VKILL]) kill_physical_line();
        /* CTRL+W */
        else if (c == CTRL_W) delete_word();

        else {
            if (!isprint((unsigned char)c) || len == MAX_LEN) {
                bell();
                continue;
            }

            buffer[len++] = c;
            buffer[len] = '\0';
        }

        redraw();
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &origg) == -1) {
        perror("tcsetattr");
        return 1;
    }

    return 0;
}
