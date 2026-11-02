#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
    pid_t pid = fork();
    if (pid == 0) {
        printf("CHILD: PID = %d, PPID = %d\n", getpid(), getppid());
        fflush(stdout);

        int status = execlp("cat", "cat", "file.txt", NULL);
        if (status == -1){
            perror("execlp");
            return 1;
        }
    }

    else if (pid > 0) {
        printf("PARENT: PID = %d, PPID = %d\n\n", getpid(), getppid());
        fflush(stdout);

        int status;
        if (waitpid(pid, &status, 0) == -1){
            perror("waitpid");
            return 1;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0){
            fprintf(stderr, "Дочерний процесс упал :(\n");
            return 1;
        }

        printf("Процесс родителя делает очень важную работу\n");
        printf("Текст\n");
        printf("Еще текст\n");
        printf("Напоследок еще печать\n");
        printf("Ладна все.\n");
    }

    else {
        perror("fork");
        return 1;
    }

    return 0;
}
