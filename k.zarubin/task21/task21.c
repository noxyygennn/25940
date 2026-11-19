#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t cnt = 0;
volatile sig_atomic_t quit = 0;

void sigintt(int sig) {
    (void)sig;
    cnt++;
    write(1, "\a", 1);
}

void sigquitt(int sig) {
    (void)sig;
    quit = 1;
}

int main(void) {
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof sig_act);
    sigemptyset(&sig_act.sa_mask);

    sig_act.sa_handler = sigintt;
    if (sigaction(SIGINT, &sig_act, NULL) == -1) {
        perror("sigaction SIGINT"); 
        return 1; 
    }

    sig_act.sa_handler = sigquitt;
    if (sigaction(SIGQUIT, &sig_act, NULL) == -1) { 
        perror("sigaction SIGQUIT"); 
        return 1; 
    }

    while (!quit){
        pause();
    }

    putchar('\n');
    printf("SIGINT пропикал %d раз(а)\n", (int)cnt);
    return 0;
}
