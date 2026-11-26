#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(fd[1]);
        char buf[101];
        ssize_t n;
        while((n = read(fd[0], buf, sizeof(buf))) > 0){
            for (int i = 0; i < n; i++){
                buf[i] = (char)toupper(buf[i]);
            }
            write(1, buf, (size_t)n);
        }
        if (n == -1){
            perror("read");
            return 1;
        }
        close(fd[0]);
    }

    else {
        close(fd[0]);
        const char *txt = "The Quick BROWN fox JuMpS over the LAZY dog;;;..\n";
        write(fd[1], txt, strlen(txt));
        close(fd[1]);
        wait(NULL);
    }
    return 0;
}
