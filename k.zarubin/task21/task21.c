#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

volatile sig_atomic_t cnt = 0;
volatile sig_atomic_t quit = 0;

void sigintt(int sig){
    (void)sig;
    cnt++;
    write(1, "\a", 1);
}

void sigquitt(int sig){
    (void)sig;
    quit = 1;
}

int main(){
    signal(SIGINT, sigintt);
    signal(SIGQUIT, sigquitt);
    while (!quit){
        pause();
    }
    putchar('\n');
    printf("SIGINT пропикал %d раз(а)\n", (int)cnt);
    return 0;
}
