/**
 * @file drone_client.h
 * @brief Drone istemcisi için gerekli yapılar ve fonksiyonlar
 */
#ifndef DRONE_CLIENT_H
#define DRONE_CLIENT_H

#include "drone.h"
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

/**
 * @brief Drone istemcisi yapısı
 */
typedef struct {
    char drone_id[32];           // Drone ID
    char session_id[64];         // Oturum ID (handshake sonrası sunucu tarafından atanır)
    int socket_fd;               // Sunucu soketi
    Coord location;              // Mevcut konum
    Coord target;                // Hedef konum (görevdeyse)
    char status[16];             // Durum (idle, busy)
    
    bool connected;              // Sunucuya bağlı mı?
    bool handshake_completed;    // İlk el sıkışma tamamlandı mı?
    
    int status_update_interval;  // Durum güncelleme aralığı (saniye)
    int heartbeat_interval;      // Heartbeat aralığı (saniye)
    
    char mission_id[32];         // Aktif görev ID (varsa)
    bool on_mission;             // Drone görev üzerinde mi?
    
    pthread_t nav_thread;        // Navigasyon thread'i
    pthread_t comms_thread;      // İletişim thread'i
    pthread_mutex_t lock;        // Drone kilit mekanizması
    
    time_t last_status_update;   // Son durum güncellemesi zamanı
    time_t last_heartbeat;       // Son heartbeat zamanı
} DroneClient;

/**
 * @brief Drone istemcisini başlatır ve sunucuya bağlanır
 * @param drone_id Drone ID
 * @param server_ip Sunucu IP adresi
 * @param server_port Sunucu port numarası
 * @return DroneClient* Başlatılan drone istemcisi, hata durumunda NULL
 */
DroneClient *drone_client_init(const char *drone_id, const char *server_ip, int server_port);

/**
 * @brief Drone istemcisini durdurur ve kaynakları temizler
 * @param client Drone istemcisi
 */
void drone_client_cleanup(DroneClient *client);

/**
 * @brief Drone'u sunucuya bağlar
 * @param client Drone istemcisi
 * @return int 0: başarılı, -1: hata
 */
int drone_client_connect(DroneClient *client);

/**
 * @brief Drone'u sunucudan ayırır
 * @param client Drone istemcisi
 */
void drone_client_disconnect(DroneClient *client);

/**
 * @brief Drone durumunu günceller ve sunucuya bildirir
 * @param client Drone istemcisi
 * @return int 0: başarılı, -1: hata
 */
int drone_client_update_status(DroneClient *client);

/**
 * @brief Sunucudan gelen mesajları işler
 * @param client Drone istemcisi
 * @return int 0: başarılı, -1: hata
 */
int drone_client_process_messages(DroneClient *client);

/**
 * @brief Drone'u hedefe doğru hareket ettirir
 * @param client Drone istemcisi
 */
void drone_client_navigate(DroneClient *client);

/**
 * @brief Görevin tamamlandığını bildirir
 * @param client Drone istemcisi
 * @param success Görevin başarılı olup olmadığı
 * @param details Detaylar
 * @return int 0: başarılı, -1: hata
 */
int drone_client_complete_mission(DroneClient *client, bool success, const char *details);

/**
 * @brief Heartbeat mesajına yanıt gönderir
 * @param client Drone istemcisi
 * @return int 0: başarılı, -1: hata
 */
int drone_client_respond_to_heartbeat(DroneClient *client);

/**
 * @brief Navigasyon thread'i
 * @param arg Drone istemcisi
 * @return void* NULL
 */
void *drone_navigation_thread(void *arg);

/**
 * @brief İletişim thread'i
 * @param arg Drone istemcisi
 * @return void* NULL
 */
void *drone_communication_thread(void *arg);

#endif /* DRONE_CLIENT_H */ 