/* aio_example.c*/
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* See notes on why "volatile sig_atomic_t" is better*/
volatile sig_atomic_t canread = 0;

void sigio_handler(int signo) {
    canread = 1;
    return;
}
void spin_for_io() {
    while (!canread) {
        // sleep(1);
    }
}


int main() {
    /*set owner for fd*/
    fcntl(0, F_SETOWN, getpid());
    /*enable SIGIO on fd*/
    fcntl(0, F_SETFL, O_ASYNC);

    /*set signal handler*/
    struct sigaction action = {0};
    action.sa_handler = &sigio_handler;

    /*you can also use sa_sigction
    action.sa_sigaction = &sigio_handler;//need to change function prototype
    */
    if (sigaction(SIGIO, &action, NULL) == -1) {
        perror("signal action");
        exit(EXIT_FAILURE);
    }

    canread = 0;
    char buf[256];

    for (;;) {
        spin_for_io(); /*you may be doing smthing else*/
        while (read(0, buf, 256) > 0) {
            printf("your msg: %.256s\n", buf);
            fflush(stdout);
        }
    }
}
