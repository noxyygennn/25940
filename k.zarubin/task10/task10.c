#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc < 2){
        fprintf(stderr, "Молодец, аргументы напиши в след раз...\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0){
        printf("CHILD: PID = %d, PPID = %d\n\n", getpid(), getppid());
        fflush(stdout);

        int status = execvp(argv[1], &argv[1]);
        if (status == -1){
            perror("execvp");
            return 1;
        }
    }
    else if (pid > 0){
        printf("PARENT: PID = %d, PPID = %d\n", getpid(), getppid());
        fflush(stdout);

        int status;
        if (waitpid(pid, &status, 0) == -1){
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status)) printf("Код завершения дочернего процесса: %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status)) printf("Сигнал завершения дочернего процесса: %d\n", WTERMSIG(status));
    }
    else {
        perror("fork");
        return 1;
    }
    return 0;
}
