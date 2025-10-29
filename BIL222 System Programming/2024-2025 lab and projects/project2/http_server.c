#include "headers/http_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 8192

// HTTP sunucu küresel değişkenleri
static int server_socket = -1;
static pthread_t http_thread;
static volatile bool http_running = false;
static char *web_root_dir = NULL;

// MIME türlerini al
static const char* get_mime_type(const char *filename) {
    char *ext = strrchr(filename, '.');
    if (!ext) return "text/plain";
    ext++; // noktayı atla
    
    if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0) return "text/html";
    if (strcasecmp(ext, "css") == 0) return "text/css";
    if (strcasecmp(ext, "js") == 0) return "application/javascript";
    if (strcasecmp(ext, "json") == 0) return "application/json";
    if (strcasecmp(ext, "png") == 0) return "image/png";
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) return "image/jpeg";
    if (strcasecmp(ext, "gif") == 0) return "image/gif";
    if (strcasecmp(ext, "svg") == 0) return "image/svg+xml";
    if (strcasecmp(ext, "ico") == 0) return "image/x-icon";
    
    return "text/plain";
}

// HTTP isteği işleme
static void handle_http_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // İsteği oku
    bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    // HTTP metodunu ve yolu ayır
    char method[16], path[512], protocol[16];
    sscanf(buffer, "%s %s %s", method, path, protocol);
    
    // Sadece GET isteklerini işle
    if (strcmp(method, "GET") != 0) {
        char response[] = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }
    
    // Kök dizin ise index.html'e yönlendir
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
    
    // Dosya yolunu oluştur
    char file_path[1024];
    sprintf(file_path, "%s%s", web_root_dir, path);
    
    // Dosyayı aç
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        // 404 Not Found
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }
    
    // Dosya boyutunu al
    struct stat file_stat;
    fstat(fd, &file_stat);
    off_t file_size = file_stat.st_size;
    
    // MIME türünü belirle
    const char *mime_type = get_mime_type(file_path);
    
    // HTTP başlığını gönder
    char header[1024];
    sprintf(header, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n", 
            mime_type, (long)file_size);
    
    send(client_socket, header, strlen(header), 0);
    
    // Dosya içeriğini gönder
    char file_buffer[4096];
    ssize_t bytes_sent;
    while ((bytes_read = read(fd, file_buffer, sizeof(file_buffer))) > 0) {
        bytes_sent = send(client_socket, file_buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            break;
        }
    }
    
    // Kaynakları temizle
    close(fd);
    close(client_socket);
}

// HTTP istemcilerini işleyen thread fonksiyonu
static void *http_client_thread(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);
    
    handle_http_request(client_socket);
    return NULL;
}

// HTTP sunucu thread fonksiyonu
static void *http_server_thread(void *arg) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    printf("HTTP sunucusu %s dizinindeki dosyaları servis ediyor\n", web_root_dir);
    
    while (http_running) {
        // Bağlantı bekle
        int *client_socket = malloc(sizeof(int));
        if (!client_socket) continue;
        
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (*client_socket < 0) {
            free(client_socket);
            continue;
        }
        
        // Yeni istemci için thread oluştur
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, http_client_thread, client_socket) != 0) {
            free(client_socket);
            close(*client_socket);
            continue;
        }
        
        // Thread'i detach et (otomatik temizlenir)
        pthread_detach(client_thread);
    }
    
    return NULL;
}

// HTTP sunucusunu başlatır
int start_http_server(int port, const char *web_root) {
    if (http_running) {
        printf("HTTP sunucusu zaten çalışıyor\n");
        return 0;
    }
    
    // Web kök dizinini kaydet
    web_root_dir = strdup(web_root);
    if (!web_root_dir) {
        perror("Web dizini kopyalanamadı");
        return -1;
    }
    
    // Sunucu soketini oluştur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("HTTP soket oluşturulamadı");
        free(web_root_dir);
        web_root_dir = NULL;
        return -1;
    }
    
    // Socket ayarlarını yapılandır
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Soket seçenekleri ayarlanamadı");
        close(server_socket);
        free(web_root_dir);
        web_root_dir = NULL;
        return -1;
    }
    
    // Adres yapılandırması
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // Soketi adrese bağla
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Soket bağlanamadı");
        close(server_socket);
        free(web_root_dir);
        web_root_dir = NULL;
        return -1;
    }
    
    // Bağlantıları dinlemeye başla
    if (listen(server_socket, MAX_CONNECTIONS) < 0) {
        perror("Soket dinlemeye başlayamadı");
        close(server_socket);
        free(web_root_dir);
        web_root_dir = NULL;
        return -1;
    }
    
    http_running = true;
    
    // HTTP sunucu thread'ini başlat
    if (pthread_create(&http_thread, NULL, http_server_thread, NULL) != 0) {
        perror("HTTP thread oluşturulamadı");
        close(server_socket);
        free(web_root_dir);
        web_root_dir = NULL;
        http_running = false;
        return -1;
    }
    
    printf("HTTP sunucusu %d portunda başlatıldı\n", port);
    return 0;
}

// HTTP sunucusunu durdurur
void stop_http_server() {
    if (!http_running) return;
    
    http_running = false;
    
    // Bağlantıyı kapat
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
    
    // Thread'i bekle ve temizle
    pthread_cancel(http_thread);
    pthread_join(http_thread, NULL);
    
    // Web kök dizinini temizle
    if (web_root_dir) {
        free(web_root_dir);
        web_root_dir = NULL;
    }
    
    printf("HTTP sunucusu durduruldu\n");
}

// HTTP sunucusunun çalışıp çalışmadığını kontrol eder
bool is_http_running() {
    return http_running;
} 