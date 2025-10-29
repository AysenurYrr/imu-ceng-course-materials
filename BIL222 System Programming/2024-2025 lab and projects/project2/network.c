#include "headers/network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    // Soket oluştur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    // Soket seçeneklerini ayarla (REUSEADDR)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        return -1;
    }
    
    // Adres bilgisi hazırla
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // Soketi porta bağla
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }
    
    // Bağlantıları dinlemeye başla
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }
    
    printf("Server is listening on port %d\n", port);
    return server_fd;
}

int create_client_socket(const char* server_ip, int port) {
    int sock;
    struct sockaddr_in serv_addr;
    
    // Soket oluştur
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    // Sunucu adres bilgisi hazırla
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // IP adresini dönüştür
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        return -1;
    }
    
    // Sunucuya bağlan
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }
    
    printf("Connected to server %s:%d\n", server_ip, port);
    return sock;
}

int receive_data(int sockfd, char* buffer, int buffer_size) {
    int bytes_received = recv(sockfd, buffer, buffer_size - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0'; // NULL ile sonlandır
    } else if (bytes_received == 0) {
        // Bağlantı kapandı
        printf("Connection closed by peer\n");
    } else {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // Timeout, veri yok
            return 0;
        } else {
            perror("Recv failed");
        }
    }
    
    return bytes_received;
}

int send_data(int sockfd, const char* buffer, int length) {
    int total_sent = 0;
    int bytes_left = length;
    int n;
    
    // Tüm veriyi gönderene kadar devam et
    while (total_sent < length) {
        n = send(sockfd, buffer + total_sent, bytes_left, 0);
        if (n == -1) {
            break;
        }
        total_sent += n;
        bytes_left -= n;
    }
    
    return (n == -1) ? -1 : total_sent;
}

int wait_for_socket(int sockfd, int timeout_sec, bool is_read, bool is_write) {
    fd_set read_fds, write_fds;
    struct timeval tv;
    int retval;
    
    // fd_set'leri sıfırla
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    
    // İzlenecek soketleri ekle
    if (is_read) FD_SET(sockfd, &read_fds);
    if (is_write) FD_SET(sockfd, &write_fds);
    
    // Timeout değerini ayarla
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    
    // select() ile soketi izle
    retval = select(sockfd + 1, 
                    is_read ? &read_fds : NULL, 
                    is_write ? &write_fds : NULL, 
                    NULL, 
                    &tv);
    
    if (retval == -1) {
        perror("Select failed");
        return -1;
    } else if (retval == 0) {
        // Timeout
        return 0;
    }
    
    // Soket hazır
    return 1;
}

void close_socket(int sockfd) {
    close(sockfd);
}

void get_client_info(struct sockaddr_in* client_addr, char* ip_buffer, int ip_buffer_size, int* port) {
    // IP adresini al
    inet_ntop(AF_INET, &client_addr->sin_addr, ip_buffer, ip_buffer_size);
    
    // Port numarasını al
    *port = ntohs(client_addr->sin_port);
}

int set_keepalive(int sockfd, bool enable) {
    int optval = enable ? 1 : 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        perror("Setsockopt (SO_KEEPALIVE) failed");
        return -1;
    }
    
    if (enable) {
        // TCP_KEEPIDLE: Bağlantı boşta kaldığında kaç saniye sonra keepalive paketleri gönderilmeye başlanacak
        int idle = 60;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) < 0) {
            perror("Setsockopt (TCP_KEEPIDLE) failed");
            return -1;
        }
        
        // TCP_KEEPINTVL: Keepalive paketleri arasındaki süre (saniye)
        int interval = 10;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0) {
            perror("Setsockopt (TCP_KEEPINTVL) failed");
            return -1;
        }
        
        // TCP_KEEPCNT: Kaç adet keepalive paketi gönderilecek
        int count = 3;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) < 0) {
            perror("Setsockopt (TCP_KEEPCNT) failed");
            return -1;
        }
    }
    
    return 0;
}

int set_socket_timeout(int sockfd, int seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    
    // Okuma için timeout ayarla
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Setsockopt (SO_RCVTIMEO) failed");
        return -1;
    }
    
    // Yazma için timeout ayarla
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Setsockopt (SO_SNDTIMEO) failed");
        return -1;
    }
    
    return 0;
} 