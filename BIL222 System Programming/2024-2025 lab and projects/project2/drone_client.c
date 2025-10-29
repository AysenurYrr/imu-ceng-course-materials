#include "headers/drone_client.h"
#include "headers/network.h"
#include "headers/json_helper.h"
#include "headers/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#define DEBUG_PRINT 0
// Global değişkenler
static volatile int client_running = 0;

// Drone istemcisinden gönderilen JSON mesajlarını yazdıran yardımcı fonksiyon
void print_json_message(const char* prefix, const char* json_str) {
    printf("\n===== %s =====\n", prefix);
    printf("%s\n", json_str);
    printf("================\n\n");
}

DroneClient *drone_client_init(const char *drone_id, const char *server_ip, int server_port) {
    // Bellek ayır
    DroneClient *client = (DroneClient *)malloc(sizeof(DroneClient));
    if (!client) {
        perror("Failed to allocate memory for drone client");
        return NULL;
    }
    
    // Varsayılan değerlerle başlat
    memset(client, 0, sizeof(DroneClient));
    
    // Drone kimliği ve özelliklerini ayarla
    strncpy(client->drone_id, drone_id, sizeof(client->drone_id) - 1);
    
    // Başlangıç konumunu rastgele ayarla - daha küçük değerler kullanarak
    srand(time(NULL) + atoi(drone_id));
    client->location.x = rand() % 40;
    client->location.y = rand() % 30;
    client->target = client->location;
    
    // Diğer parametreleri ayarla
    strcpy(client->status, DRONE_STATUS_IDLE);
    client->on_mission = false;
    client->connected = false;
    client->handshake_completed = false;
    
    // Varsayılan aralıklar
    client->status_update_interval = DEFAULT_STATUS_UPDATE_INTERVAL;
    client->heartbeat_interval = DEFAULT_HEARTBEAT_INTERVAL;
    
    // Zaman damgaları
    client->last_status_update = time(NULL);
    client->last_heartbeat = time(NULL);
    
    // Mutex başlat
    pthread_mutex_init(&client->lock, NULL);
    
    // Sunucuya bağlan
    client->socket_fd = create_client_socket(server_ip, server_port);
    if (client->socket_fd < 0) {
        pthread_mutex_destroy(&client->lock);
        free(client);
        return NULL;
    }
    
    client->connected = true;
    client_running = 1;
    
    return client;
}

void drone_client_cleanup(DroneClient *client) {
    if (!client) return;
    
    client_running = 0;
    
    // Thread'leri sonlandır
    if (client->nav_thread) {
        pthread_cancel(client->nav_thread);
        pthread_join(client->nav_thread, NULL);
    }
    
    if (client->comms_thread) {
        pthread_cancel(client->comms_thread);
        pthread_join(client->comms_thread, NULL);
    }
    
    // Drone'u server'dan ayır ve global listeden kaldırılması için bildir
    if (client->connected) {
        drone_client_disconnect(client);
    }
    
    // Mutex'i temizle
    pthread_mutex_destroy(&client->lock);
    
    // Belleği serbest bırak
    free(client);
    
    printf("Drone client cleaned up\n");
}

int drone_client_connect(DroneClient *client) {
    // Eğer zaten bağlıysa, direkt dön
    if (client->connected && client->handshake_completed) {
        return 0;
    }
    
    // Handshake gönder
    json_object *handshake = create_handshake_msg(client->drone_id);
    
    const char *handshake_str = json_object_to_json_string(handshake);
    int result = send_data(client->socket_fd, handshake_str, strlen(handshake_str));
    json_object_put(handshake);
    
    if (result <= 0) {
        perror("Failed to send handshake");
        return -1;
    }
    
    printf("Handshake sent for drone %s\n", client->drone_id);
    
    // Handshake yanıtını bekle - timeout azalt
    char buffer[DEFAULT_BUFFER_SIZE];
    int ready = wait_for_socket(client->socket_fd, 1, true, false); // 5s -> 1s
    if (ready <= 0) {
        perror("Timeout waiting for handshake response");
        return -1;
    }
    
    int bytes_received = receive_data(client->socket_fd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        perror("Failed to receive handshake response");
        return -1;
    }
    
    // Mesaj tipini kontrol et
    const char *msg_type = get_message_type(buffer);
    if (!msg_type || strcmp(msg_type, MSG_HANDSHAKE_ACK) != 0) {
        fprintf(stderr, "Expected HANDSHAKE_ACK but received: %s\n", msg_type ? msg_type : "unknown");
        free((void *)msg_type);
        return -1;
    }
    
    free((void *)msg_type);
    
    // JSON yanıtını çözümle
    json_object *jobj = json_tokener_parse(buffer);
    if (!jobj) {
        fprintf(stderr, "Invalid JSON in handshake response\n");
        return -1;
    }
    
    // Session ID'yi al
    json_object *jsession_id;
    if (!json_object_object_get_ex(jobj, "session_id", &jsession_id)) {
        fprintf(stderr, "No session_id in handshake response\n");
        json_object_put(jobj);
        return -1;
    }
    
    // Config ayarlarını al
    json_object *jconfig;
    if (!json_object_object_get_ex(jobj, "config", &jconfig)) {
        fprintf(stderr, "No config in handshake response\n");
        json_object_put(jobj);
        return -1;
    }
    
    // Status update interval ve heartbeat interval değerlerini al
    json_object *jstatus_interval, *jheartbeat_interval;
    if (!json_object_object_get_ex(jconfig, "status_update_interval", &jstatus_interval) ||
        !json_object_object_get_ex(jconfig, "heartbeat_interval", &jheartbeat_interval)) {
        fprintf(stderr, "Missing interval values in config\n");
        json_object_put(jobj);
        return -1;
    }
    
    // Drone'un yapılandırmasını güncelle
    pthread_mutex_lock(&client->lock);
    strncpy(client->session_id, json_object_get_string(jsession_id), sizeof(client->session_id) - 1);
    client->status_update_interval = 1; // Hızlı güncellemeler için override et
    client->heartbeat_interval = json_object_get_int(jheartbeat_interval);
    client->handshake_completed = true;
    pthread_mutex_unlock(&client->lock);
    
    json_object_put(jobj);
    
    printf("Handshake completed for drone %s (Session: %s)\n", 
           client->drone_id, client->session_id);
    
    // İlk durum güncellemesini hemen gönder
    if (drone_client_update_status(client) < 0) {
        fprintf(stderr, "Error sending initial status update\n");
        // Devam et, kritik değil
    }
    
    // Thread'leri başlat
    if (pthread_create(&client->nav_thread, NULL, drone_navigation_thread, client) != 0) {
        perror("Failed to create navigation thread");
        return -1;
    }
    
    if (pthread_create(&client->comms_thread, NULL, drone_communication_thread, client) != 0) {
        perror("Failed to create communication thread");
        pthread_cancel(client->nav_thread);
        pthread_join(client->nav_thread, NULL);
        return -1;
    }
    
    return 0;
}

void drone_client_disconnect(DroneClient *client) {
    if (!client->connected) {
        return;
    }
    
    // Önce bir DISCONNECT mesajı göndermeyi dene
    // Bu mesaj, sunucuya bu drone'un global drones listesinden silinmesi gerektiğini bildirecek
    json_object *disconnect_msg = json_object_new_object();
    json_object_object_add(disconnect_msg, "type", json_object_new_string(MSG_DISCONNECT));
    json_object_object_add(disconnect_msg, "drone_id", json_object_new_string(client->drone_id));
    json_object_object_add(disconnect_msg, "timestamp", json_object_new_int64(time(NULL)));
    
    const char *disconnect_str = json_object_to_json_string(disconnect_msg);
    
    // Mesajı göndermeyi dene, ancak başarısız olursa devam et
    // Sunucu zaten bağlantı kesintisini algılayıp gerekli işlemleri yapabilir
    if (DEBUG_PRINT) {
        print_json_message("CLIENT - DISCONNECT GÖNDER", disconnect_str);
    }
    send_data(client->socket_fd, disconnect_str, strlen(disconnect_str));
    json_object_put(disconnect_msg);
    
    // Soket bağlantısını kapat
    client->connected = false;
    close_socket(client->socket_fd);
    
    printf("Drone %s disconnected from server\n", client->drone_id);
}

int drone_client_update_status(DroneClient *client) {
    pthread_mutex_lock(&client->lock);
    
    printf("[DEBUG] Drone %s konum güncelleniyor: (%d,%d), durum: %s\n", 
           client->drone_id, client->location.x, client->location.y, 
           client->on_mission ? DRONE_STATUS_BUSY : DRONE_STATUS_IDLE);
    
    // Durum mesajı oluştur
    json_object *status = create_status_update_msg(
        client->drone_id,
        client->location,
        client->on_mission ? DRONE_STATUS_BUSY : DRONE_STATUS_IDLE
    );
    
    const char *status_str = json_object_to_json_string(status);
    
    // JSON mesajını yazdır
    if (DEBUG_PRINT) {
        print_json_message("CLIENT - STATUS UPDATE GÖNDER", status_str);
    }
    
    int result = send_data(client->socket_fd, status_str, strlen(status_str));
    json_object_put(status);
    
    if (result > 0) {
        client->last_status_update = time(NULL);
        printf("[DEBUG] Drone %s durum güncellemesi başarılı\n", client->drone_id);
    } else {
        printf("[DEBUG] HATA: Drone %s durum güncellemesi başarısız\n", client->drone_id);
    }
    
    pthread_mutex_unlock(&client->lock);
    
    return (result > 0) ? 0 : -1;
}

int drone_client_process_messages(DroneClient *client) {
    char buffer[DEFAULT_BUFFER_SIZE];
    
    // Soketin okuma için hazır olup olmadığını kontrol et
    int ready = wait_for_socket(client->socket_fd, 0, true, false);
    if (ready <= 0) {
        return 0; // Veri yok veya timeout
    }
    
    // Veri al
    int bytes_received = receive_data(client->socket_fd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            // Timeout
            return 0;
        } else {
            // Hata
            perror("Failed to receive data");
            return -1;
        }
    }
    
    // Alınan JSON mesajını yazdır
    if (DEBUG_PRINT) {
        print_json_message("CLIENT - MESAJ ALINDI", buffer);
    }
    
    // Mesaj tipini çözümle
    const char *msg_type = get_message_type(buffer);
    if (!msg_type) {
        fprintf(stderr, "Invalid message format\n");
        return -1;
    }
    
    // Mesaj tipine göre işlem yap
    if (strcmp(msg_type, MSG_ASSIGN_MISSION) == 0) {
        // Görev atama mesajını çözümle
        char mission_id[32];
        char priority[16];
        Coord target;
        
        if (parse_assign_mission_msg(buffer, mission_id, priority, &target) != 0) {
            fprintf(stderr, "Invalid ASSIGN_MISSION message\n");
            free((void *)msg_type);
            return -1;
        }
        
        // Görev bilgilerini güncelle
        pthread_mutex_lock(&client->lock);
        strncpy(client->mission_id, mission_id, sizeof(client->mission_id) - 1);
        client->target = target;
        client->on_mission = true;
        strcpy(client->status, DRONE_STATUS_BUSY);
        pthread_mutex_unlock(&client->lock);
        
        printf("Drone %s assigned mission %s to target (%d,%d) with priority %s\n",
               client->drone_id, mission_id, target.x, target.y, priority);
        
    } else if (strcmp(msg_type, MSG_HEARTBEAT) == 0) {
        // Heartbeat yanıtı gönder
        drone_client_respond_to_heartbeat(client);
        
    } else if (strcmp(msg_type, MSG_ERROR) == 0) {
        // Hata mesajını çözümle
        json_object *jobj = json_tokener_parse(buffer);
        if (!jobj) {
            fprintf(stderr, "Invalid JSON in error message\n");
            free((void *)msg_type);
            return -1;
        }
        
        json_object *jcode, *jmessage;
        if (!json_object_object_get_ex(jobj, "code", &jcode) ||
            !json_object_object_get_ex(jobj, "message", &jmessage)) {
            fprintf(stderr, "Missing fields in error message\n");
            json_object_put(jobj);
            free((void *)msg_type);
            return -1;
        }
        
        printf("Received error from server: code=%d, message=%s\n",
               json_object_get_int(jcode), json_object_get_string(jmessage));
        
        json_object_put(jobj);
    } else {
        printf("Received unknown message type: %s\n", msg_type);
    }
    
    free((void *)msg_type);
    return 0;
}

void drone_client_navigate(DroneClient *client) {
    pthread_mutex_lock(&client->lock);
    
    if (client->on_mission) {
        // Mevcut konumu kaydet
        int old_x = client->location.x;
        int old_y = client->location.y;
        
        // Hedefe doğru hareket et
        if (client->location.x < client->target.x) client->location.x++;
        else if (client->location.x > client->target.x) client->location.x--;
        
        if (client->location.y < client->target.y) client->location.y++;
        else if (client->location.y > client->target.y) client->location.y--;
        
        // Konum değiştiyse bildir
        if (old_x != client->location.x || old_y != client->location.y) {
            printf("[DEBUG] Drone %s hareket etti: (%d,%d) -> (%d,%d), hedef: (%d,%d)\n", 
                   client->drone_id, old_x, old_y, 
                   client->location.x, client->location.y,
                   client->target.x, client->target.y);
            
            // Hemen durum güncellemesi gönder
            pthread_mutex_unlock(&client->lock);
            drone_client_update_status(client);
            pthread_mutex_lock(&client->lock);
        }
        
        // Hedefe ulaşıldı mı kontrol et
        if (client->location.x == client->target.x && client->location.y == client->target.y) {
            printf("Drone %s reached target (%d,%d)\n", 
                   client->drone_id, client->target.x, client->target.y);
            
            // Görevi tamamla
            client->on_mission = false;
            
            pthread_mutex_unlock(&client->lock);
            
            // Hedefe ulaştıktan sonra kısa bir süre bekle
            // Bu, STATUS_UPDATE ve MISSION_COMPLETE mesajlarının sunucu tarafından ayrı ayrı işlenmesine yardımcı olur
            usleep(500000); // 500ms bekle
            
            // Görev tamamlama mesajı gönder - survivor koordinatlarını da ekle
            drone_client_complete_mission(client, true, "Target reached successfully");
            return;
        }
    }
    
    pthread_mutex_unlock(&client->lock);
}

int drone_client_complete_mission(DroneClient *client, bool success, const char *details) {
    pthread_mutex_lock(&client->lock);
    
    // Görev tamamlama mesajı oluştur - survivor koordinatlarını da ekle
    json_object *complete = json_object_new_object();
    json_object_object_add(complete, "type", json_object_new_string(MSG_MISSION_COMPLETE));
    json_object_object_add(complete, "drone_id", json_object_new_string(client->drone_id));
    json_object_object_add(complete, "mission_id", json_object_new_string(client->mission_id));
    json_object_object_add(complete, "success", json_object_new_boolean(success));
    json_object_object_add(complete, "details", json_object_new_string(details));

    // Hedef koordinatları ekle
    json_object *target = json_object_new_object();
    json_object_object_add(target, "x", json_object_new_int(client->target.x));
    json_object_object_add(target, "y", json_object_new_int(client->target.y));
    json_object_object_add(complete, "target", target);

    const char *complete_str = json_object_to_json_string(complete);

    // JSON mesajını yazdır
    if (DEBUG_PRINT) {
        print_json_message("CLIENT - MISSION COMPLETE GÖNDER", complete_str);
    }

    // STATUS_UPDATE ile MISSION_COMPLETE mesajları arasında kısa bir bekleme ekle
    // Bu, sunucunun mesajları ayrı ayrı işlemesini sağlayacak
    pthread_mutex_unlock(&client->lock);
    usleep(200000); // 200ms bekle
    pthread_mutex_lock(&client->lock);

    int result = send_data(client->socket_fd, complete_str, strlen(complete_str));
    json_object_put(complete);

    if (result > 0) {
        // Drone durumunu güncelle
        strcpy(client->status, DRONE_STATUS_IDLE);
        memset(client->mission_id, 0, sizeof(client->mission_id));
        client->on_mission = false;
        
        printf("Mission completion sent for drone %s at target (%d,%d)\n", 
               client->drone_id, client->target.x, client->target.y);
    }
    
    pthread_mutex_unlock(&client->lock);
    
    return (result > 0) ? 0 : -1;
}

int drone_client_respond_to_heartbeat(DroneClient *client) {
    pthread_mutex_lock(&client->lock);
    
    // Heartbeat yanıtı oluştur
    json_object *response = create_heartbeat_response_msg(client->drone_id);
    
    const char *response_str = json_object_to_json_string(response);
    int result = send_data(client->socket_fd, response_str, strlen(response_str));
    json_object_put(response);
    
    if (result > 0) {
        client->last_heartbeat = time(NULL);
    }
    
    pthread_mutex_unlock(&client->lock);
    
    return (result > 0) ? 0 : -1;
}

void *drone_navigation_thread(void *arg) {
    DroneClient *client = (DroneClient *)arg;
    
    printf("[DEBUG] Drone %s navigasyon thread'i başladı\n", client->drone_id);
    
    while (client_running && client->connected) {
        // Drone'u hedefe doğru hareket ettir
        drone_client_navigate(client);
        
        // Kısa bir süre bekle - 100ms yerine 50ms (daha hızlı hareket)
        usleep(50000); // 50ms
    }
    
    printf("[DEBUG] Drone %s navigasyon thread'i sonlandı\n", client->drone_id);
    return NULL;
}

void *drone_communication_thread(void *arg) {
    DroneClient *client = (DroneClient *)arg;
    
    printf("[DEBUG] Drone %s iletişim thread'i başladı\n", client->drone_id);
    
    // İlk durum güncellemesini hemen gönder
    drone_client_update_status(client);
    
    while (client_running && client->connected) {
        // Sunucudan gelen mesajları işle
        if (drone_client_process_messages(client) < 0) {
            fprintf(stderr, "Error processing messages\n");
            break;
        }
        
        // Durum güncellemesinin zamanı geldi mi kontrol et
        time_t now = time(NULL);
        if (now - client->last_status_update >= client->status_update_interval) {
            printf("[DEBUG] Drone %s durum güncellemesi zamanı geldi\n", client->drone_id);
            if (drone_client_update_status(client) < 0) {
                fprintf(stderr, "Error updating drone status\n");
                break;
            }
        }
        
        // Kısa bir süre bekle
        usleep(10000); // 50ms -> 10ms
    }
    
    printf("[DEBUG] Drone %s iletişim thread'i sonlandı\n", client->drone_id);
    return NULL;
} 