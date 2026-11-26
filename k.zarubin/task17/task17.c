
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define STR_LEN 40
#define MAX_LEN 1024
#define CTRL_D 0x04
#define CTRL_W 0x17
#define CTRL_C 0x03

int main() {
    struct termios coppy, origg;

    if (tcgetattr(STDIN_FILENO, &origg) == -1) {
        perror("tcgetattr");
        return 1;
    }

    coppy = origg;
    coppy.c_lflag &= ~(ECHO | ICANON | ISIG);
    coppy.c_iflag &= ~(IXON | IXOFF); /* при нажатии CTRL+S / CTRL+Q программа вставала */
    coppy.c_cc[VMIN] = 1;
    coppy.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &coppy) == -1) {
        perror("tcsetattr");
        return 1;
    }

    int lenn = 0, rows = 1, change_fl = 0;

    unsigned char c;
    char line[MAX_LEN + 1] = "";

    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1) break;
        change_fl = 0;

        if (c == CTRL_D) { /* CTRL+D */
            if (lenn == 0) break;
            write(STDOUT_FILENO, "\a", 1);
            continue;
        }

        if (c == '\n' || c == '\r') { /* ENTER */
            write(STDOUT_FILENO, "\r\n", 2);
            lenn = 0;
            line[0] = '\0';
            rows = 1;

            continue;
        }

        if (c == origg.c_cc[VERASE]) { /* ERASE (Backspace) */
            if (lenn > 0) {
                lenn--;
                line[lenn] = '\0';
                change_fl = 1;
            }
            else write(STDOUT_FILENO, "\a", 1);
        }

        else if (c == origg.c_cc[VKILL]) { /* KILL */
            lenn = 0;
            line[0] = '\0';
            change_fl = 1;
        }

        else if (c == CTRL_W) { /* CTRL+W */
            while (lenn > 0 && line[lenn - 1] == ' ') { lenn--; }
            while (lenn > 0 && line[lenn - 1] != ' ') { lenn--; }

            line[lenn] = '\0';
            change_fl = 1;
        }

        else { /* обычный символ */
            if (!isprint((unsigned char)c) || lenn == MAX_LEN || c == CTRL_C) {
                write(STDOUT_FILENO, "\a", 1);
                continue;
            }

            line[lenn++] = c;
            line[lenn] = '\0';
            change_fl = 1;
        }
        if (!change_fl) continue;

        /* очищаем старое отображение */
        for (int i = 0; i < rows; i++) {
            write(STDOUT_FILENO, "\r\033[2K", 5);
            if (i + 1 < rows) write(STDOUT_FILENO, "\033[1A", 4);
        }

        /* повторно выводим логическую строку */
        int cursor = 0;
        rows = 1;

        for (int i = 0; i < lenn;) {
            if (line[i] == ' ') { /* пробел */
                if (cursor < STR_LEN) {
                    write(STDOUT_FILENO, " ", 1);
                    cursor++;
                }
                i++;
                continue;
            }

            /* определяем длину следующего слова */
            int start = i;
            while (i < lenn && line[i] != ' ') { i++; }
            int word_lenn = i - start;

            /* проверяем, помещается ли слово */
            if (cursor > 0 && cursor + word_lenn > STR_LEN) {
                write(STDOUT_FILENO, "\r\n", 2);
                cursor = 0;
                rows++;
            }

            /* выводим слово */
            for (int j = start; j < i; j++) {
                if (cursor == STR_LEN) {
                    write(STDOUT_FILENO, "\r\n", 2);
                    cursor = 0;
                    rows++;
                }

                write(STDOUT_FILENO, &line[j], 1);
                cursor++;
            }
        }
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &origg) == -1) {
        perror("tcsetattr");
        return 1;
    }

    return 0;
}
