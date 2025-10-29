#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MSG_INTERVAL 2  // Seconds between messages
#define MAX_MSGS 5      // Messages per client

void client_process(int client_id) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client %d connected to server\n", client_id);

    // Send/receive loop
    char buf[1024];
    for (int i = 0; i < MAX_MSGS; i++) {
        // Send message
        int len = snprintf(buf, sizeof(buf), "Client %d: Message %d", client_id, i+1);
        write(sockfd, buf, len);
        printf("Client %d sent: %s\n", client_id, buf);

        // Receive echo
        ssize_t bytes = read(sockfd, buf, sizeof(buf));
        if (bytes <= 0) {
            printf("Client %d: Server disconnected\n", client_id);
            break;
        }
        buf[bytes] = '\0';
        printf("Client %d received: %s\n", client_id, buf);

        sleep(MSG_INTERVAL);
    }

    close(sockfd);
    printf("Client %d exiting\n", client_id);
}

int main(int argc, char *argv[]) {
    int num_clients = 1;

    if (argc > 1) {
        num_clients = atoi(argv[1]);
        if (num_clients <= 0) {
            fprintf(stderr, "Invalid client count. Using 1 client.\n");
            num_clients = 1;
        }
    }

    // Spawn client processes
    for (int i = 0; i < num_clients; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            client_process(i + 1);  // Child process
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all children to exit
    while (wait(NULL) > 0);
    printf("All clients finished\n");
    return 0;
}
