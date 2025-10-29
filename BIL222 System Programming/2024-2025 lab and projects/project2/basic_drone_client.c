#include "headers/drone_client.h"
#include "headers/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// Global değişkenler
volatile int client_should_exit = 0;
DroneClient *client = NULL;

// Sinyal işleyici
void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nReceived SIGINT, shutting down client...\n");
        client_should_exit = 1;
        
        // Hemen kaynakları temizle ve çık
        if (client) {
            drone_client_cleanup(client);
            printf("Client shutdown complete\n");
        }
        
        // Programı sonlandır
        exit(EXIT_SUCCESS);
    }
}

void print_usage(const char *program_name) {
    printf("Usage: %s <drone_id> <server_ip> [port]\n", program_name);
    printf("  drone_id: Unique identifier for this drone\n");
    printf("  server_ip: IP address of the server\n");
    printf("  port: Server port (default: %d)\n", DEFAULT_SERVER_PORT);
}

int main(int argc, char *argv[]) {
    // Parametre kontrolü
    if (argc < 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *drone_id = argv[1];
    const char *server_ip = argv[2];
    
    int port = (argc > 3) ? atoi(argv[3]) : DEFAULT_SERVER_PORT;
    
    // Parametre değerlerini doğrula
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number. Using default: %d\n", DEFAULT_SERVER_PORT);
        port = DEFAULT_SERVER_PORT;
    }
    
    // Sinyal işleyici
    signal(SIGINT, signal_handler);
    
    printf("Starting drone client %s, connecting to %s:%d...\n", drone_id, server_ip, port);
    
    // Drone istemcisini başlat
    client = drone_client_init(drone_id, server_ip, port);
    if (!client) {
        fprintf(stderr, "Failed to initialize drone client\n");
        return EXIT_FAILURE;
    }
    
    // Sunucuya bağlan ve handshake gönder
    if (drone_client_connect(client) != 0) {
        fprintf(stderr, "Failed to connect to server\n");
        drone_client_cleanup(client);
        return EXIT_FAILURE;
    }
    
    printf("Connected to server. Press Ctrl+C to exit.\n");
    
    // Ana döngü - thread'ler mesajları ve navigasyonu işleyecek
    while (!client_should_exit) {
        // Sadece bekle, işi thread'ler yapıyor
        sleep(1);
    }
    
    printf("Disconnecting from server...\n");
    
    // Kaynakları temizle
    drone_client_cleanup(client);
    
    printf("Client shutdown complete\n");
    
    return 0;
} 