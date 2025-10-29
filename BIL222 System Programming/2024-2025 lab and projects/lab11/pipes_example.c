/*
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    int fd[2];
    /* Called with the O_CLOEXEC flag to prevent any commands from blocking */
    pipe(fd);
    fcntl(fd[0], F_SETFL, O_NONBLOCK);

    printf("started\n");
    pid_t p = fork();
    if (p > 0) {
        int score;
        printf("started\n");

        char buf[1024] = {0};
        read(fd[0], buf, 1024);
        printf("read did not wait, because of O_NONBLOCK\n");
        printf("The child says: %.1024s\n", buf);
    } else {
        sleep(1);/*sleep to finish after parent*/
        char msg[1024] = {0};
        sprintf(msg, "Score is %d", 10 + 10);
        write(fd[1], msg, 1024);
        printf("child done\n");
    }
    
    close(fd[0]);
    close(fd[1]);
    return 0;
}
