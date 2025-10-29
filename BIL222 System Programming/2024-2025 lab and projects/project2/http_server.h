/**
 * @file http_server.h
 * @brief Basit HTTP sunucu modülü
 */
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdbool.h>
#include <pthread.h>

/**
 * @brief HTTP sunucusunu başlatır
 * @param port Dinlenecek port numarası
 * @param web_root Web dosyalarının kök dizini
 * @return int 0: başarılı, -1: hata
 */
int start_http_server(int port, const char *web_root);

/**
 * @brief HTTP sunucusunu durdurur
 */
void stop_http_server();

/**
 * @brief HTTP sunucusunun çalışıp çalışmadığını kontrol eder
 * @return bool Çalışıyor mu?
 */
bool is_http_running();

#endif /* HTTP_SERVER_H */ 