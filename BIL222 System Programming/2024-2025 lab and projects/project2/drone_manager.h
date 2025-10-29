/**
 * @file drone_manager.h
 * @brief Sunucu tarafında drone'ları yönetmek için gerekli yapılar ve fonksiyonlar
 */
#ifndef DRONE_MANAGER_H
#define DRONE_MANAGER_H

#include "drone.h"
#include "list.h"
#include <pthread.h>
#include <time.h>

/**
 * @brief Sunucu tarafında drone bilgilerini saklayan yapı
 */
typedef struct {
    char drone_id[32];           // Drone ID
    char session_id[64];         // Oturum ID
    int socket_fd;               // Soket tanımlayıcısı
    Coord location;              // Konum
    char status[16];             // Durum (idle, busy, charging)
    time_t last_heartbeat;       // Son heartbeat zamanı
    int missed_heartbeats;       // Kaçırılan heartbeat sayısı
    pthread_t thread_id;         // İstemci thread'i
    pthread_mutex_t lock;        // Drone kilit mekanizması
    char mission_id[32];         // Aktif görev ID (varsa)
    Coord target;                // Hedef konum (görevdeyse)
} ServerDrone;

/**
 * @brief Drone sunucusunu başlatır
 * @param port Dinlenecek port numarası
 * @return int 0: başarılı, -1: hata
 */
int start_drone_server(int port);

/**
 * @brief Drone sunucusunu durdurur
 */
void stop_drone_server(void);

/**
 * @brief Bağlı istemcileri işler ve istekleri karşılar
 */
void handle_connections(void);

/**
 * @brief Belirli bir dronel'a görev atar
 * @param drone Drone
 * @param target Hedef koordinatlar
 * @param priority Öncelik (low, medium, high)
 * @return int 0: başarılı, -1: hata
 */
int assign_mission_to_drone(ServerDrone *drone, Coord target, const char *priority);

/**
 * @brief Bağlı tüm drone'lara heartbeat mesajı gönderir
 * @return int Başarılı gönderilen heartbeat sayısı
 */
int send_heartbeats_to_all(void);

/**
 * @brief Drone bağlantısını kapatır ve kaynakları temizler
 * @param drone Drone
 */
void disconnect_drone(ServerDrone *drone);

/**
 * @brief Bağlı drone'ların durumlarını yazdırır
 */
void print_drone_status(void);

/**
 * @brief Oturum ID'si oluşturur
 * @param buffer ID'nin yazılacağı tampon
 * @param size Tampon boyutu
 */
void generate_session_id(char *buffer, size_t size);

/**
 * @brief Görev ID'si oluşturur
 * @param buffer ID'nin yazılacağı tampon
 * @param size Tampon boyutu
 */
void generate_mission_id(char *buffer, size_t size);

/**
 * @brief Drone yöneticisini başlatır
 * @return int 0: başarılı, -1: hata
 */
int init_drone_manager(void);

/**
 * @brief Drone yöneticisini temizler
 */
void cleanup_drone_manager(void);

#endif /* DRONE_MANAGER_H */ 