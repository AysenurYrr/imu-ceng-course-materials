/**
 * @file websocket_server.h
 * @brief WebSocket sunucu modülü
 */
#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <stdbool.h>
#include <pthread.h>

/**
 * @brief WebSocket sunucusunu başlatır
 * @param port Dinlenecek port numarası
 * @return int 0: başarılı, -1: hata
 */
int start_websocket_server(int port);

/**
 * @brief WebSocket sunucusunu durdurur
 */
void stop_websocket_server();

/**
 * @brief Tüm web istemcilerine JSON mesajı gönderir
 * @param json_message Gönderilecek JSON mesajı
 */
void websocket_broadcast(const char *json_message);

/**
 * @brief WebSocket sunucusunun çalışıp çalışmadığını kontrol eder
 * @return bool Çalışıyor mu?
 */
bool is_websocket_running();

/**
 * @brief Yeni bir drone başlatma isteğini işler
 * @param drone_id Drone ID
 * @param server_ip Sunucu IP adresi
 * @param server_port Sunucu port numarası
 * @return int 0: başarılı, -1: hata 
 */
int launch_drone_client(const char *drone_id, const char *server_ip, int server_port);

#endif /* WEBSOCKET_SERVER_H */ 