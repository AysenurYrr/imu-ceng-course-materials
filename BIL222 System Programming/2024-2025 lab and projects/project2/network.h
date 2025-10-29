/**
 * @file network.h
 * @brief Ağ iletişimi için yardımcı fonksiyonlar
 */
#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

/**
 * @brief TCP sunucu soketi oluşturur ve belirtilen porta bağlar
 * @param port Dinlenecek port numarası
 * @return int Oluşturulan soket tanımlayıcısı, hata durumunda -1
 */
int create_server_socket(int port);

/**
 * @brief TCP istemci soketi oluşturur ve belirtilen sunucuya bağlanır
 * @param server_ip Sunucu IP adresi
 * @param port Sunucu port numarası
 * @return int Oluşturulan soket tanımlayıcısı, hata durumunda -1
 */
int create_client_socket(const char* server_ip, int port);

/**
 * @brief Belirtilen soketten veri alır
 * @param sockfd Soket tanımlayıcısı
 * @param buffer Alınan verilerin yazılacağı tampon
 * @param buffer_size Tampon boyutu
 * @return int Alınan veri boyutu, hata durumunda -1, bağlantı kapatıldığında 0
 */
int receive_data(int sockfd, char* buffer, int buffer_size);

/**
 * @brief Belirtilen sokete veri gönderir
 * @param sockfd Soket tanımlayıcısı
 * @param buffer Gönderilecek veri tamponu
 * @param length Gönderilecek veri boyutu
 * @return int Gönderilen veri boyutu, hata durumunda -1
 */
int send_data(int sockfd, const char* buffer, int length);

/**
 * @brief Soketi belirli bir süre için izler
 * @param sockfd Soket tanımlayıcısı
 * @param timeout_sec Zaman aşımı süresi (saniye)
 * @param is_read Okuma için izleme
 * @param is_write Yazma için izleme
 * @return int 1: hazır, 0: zaman aşımı, -1: hata
 */
int wait_for_socket(int sockfd, int timeout_sec, bool is_read, bool is_write);

/**
 * @brief Soketi kapatır
 * @param sockfd Soket tanımlayıcısı
 */
void close_socket(int sockfd);

/**
 * @brief İstemci adresinden IP ve port bilgisini alır
 * @param client_addr İstemci adresi
 * @param ip_buffer IP adresi için tampon
 * @param ip_buffer_size IP tamponu boyutu
 * @param port Port numarası için pointer
 */
void get_client_info(struct sockaddr_in* client_addr, char* ip_buffer, int ip_buffer_size, int* port);

/**
 * @brief Soketi, bağlantı koptuğunda hemen kapatmak yerine beklemek için yapılandırır
 * @param sockfd Soket tanımlayıcısı
 * @param enable Etkinleştir/Devre dışı bırak
 * @return int 0: başarılı, -1: hata
 */
int set_keepalive(int sockfd, bool enable);

/**
 * @brief Soketin timeout değerlerini ayarlar
 * @param sockfd Soket tanımlayıcısı
 * @param seconds Saniye
 * @return int 0: başarılı, -1: hata
 */
int set_socket_timeout(int sockfd, int seconds);

#endif /* NETWORK_H */ 