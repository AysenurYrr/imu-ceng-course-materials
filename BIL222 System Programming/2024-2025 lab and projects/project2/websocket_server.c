#include "headers/websocket_server.h"
#include "headers/drone_client.h"
#include "headers/json_helper.h"
#include "headers/globals.h"
#include "headers/protocol.h"
#include "headers/drone.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include <spawn.h>
#include <sys/wait.h>

// WebSocket bağlantıları için maksimum sayı
#define MAX_CLIENTS 100

// WebSocket protokolü adı

// WebSocket küresel değişkenleri
static struct lws_context *context;
static pthread_t ws_thread;
static volatile bool ws_running = false;
static int client_count = 0;
static struct lws *clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Tüm drone ve survivor durumlarını içeren JSON mesajını oluşturur
static char *create_status_json();

// WebSocket istemcilerine veri gönderme
static void send_to_client(struct lws *wsi, const char *message, size_t len) {
    unsigned char *buf = malloc(LWS_PRE + len);
    if (!buf) return;
    
    memcpy(buf + LWS_PRE, message, len);
    lws_write(wsi, buf + LWS_PRE, len, LWS_WRITE_TEXT);
    free(buf);
}

// Callback - WebSocket olaylarını işler
static int callback_drone_protocol(struct lws *wsi, enum lws_callback_reasons reason,
                                  void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            // Yeni bir istemci bağlandı
            pthread_mutex_lock(&clients_mutex);
            if (client_count < MAX_CLIENTS) {
                clients[client_count++] = wsi;
                printf("Yeni web istemcisi bağlandı. Toplam: %d\n", client_count);
                
                // Güncel durumu hemen gönder
                char *status_json = create_status_json();
                if (status_json) {
                    send_to_client(wsi, status_json, strlen(status_json));
                    free(status_json);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
        
        case LWS_CALLBACK_CLOSED: {
            // İstemci bağlantısı kapandı
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == wsi) {
                    // İstemciyi listeden çıkar
                    for (int j = i; j < client_count - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    client_count--;
                    printf("Web istemcisi ayrıldı. Kalan: %d\n", client_count);
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
        
        case LWS_CALLBACK_RECEIVE: {
            // İstemciden veri alındı
            char *data = (char *)in;
            data[len] = '\0';  // Null-terminate
            
            // JSON mesajını ayrıştır
            json_object *jobj = json_tokener_parse(data);
            if (!jobj) {
                printf("Geçersiz JSON formatı: %s\n", data);
                break;
            }
            
            // Mesaj tipini al
            json_object *jtype;
            if (json_object_object_get_ex(jobj, "type", &jtype)) {
                const char *msg_type = json_object_get_string(jtype);
                
                // "launch_drone" mesajını işle
                if (strcmp(msg_type, "launch_drone") == 0) {
                    json_object *jdrone_id, *jserver_ip, *jserver_port;
                    
                    if (json_object_object_get_ex(jobj, "drone_id", &jdrone_id) &&
                        json_object_object_get_ex(jobj, "server_ip", &jserver_ip) &&
                        json_object_object_get_ex(jobj, "server_port", &jserver_port)) {
                        
                        const char *drone_id = json_object_get_string(jdrone_id);
                        const char *server_ip = json_object_get_string(jserver_ip);
                        int server_port = json_object_get_int(jserver_port);
                        
                        // Yeni drone başlat
                        launch_drone_client(drone_id, server_ip, server_port);
                    }
                }
                // Diğer mesaj tipleri buraya eklenebilir
            }
            
            json_object_put(jobj);
            break;
        }
        
        default:
            break;
    }
    
    return 0;
}

// WebSocket protokolü tanımı
static struct lws_protocols protocols[] = {
    {
        .name = "drone-protocol",
        .callback = callback_drone_protocol,
        .per_session_data_size = 0,
        .rx_buffer_size = 1024,
    },
    { NULL, NULL, 0, 0 } // Son protokol sıfır olmalı
};

// WebSocket thread fonksiyonu
static void *websocket_thread(void *arg) {
    while (ws_running) {
        lws_service(context, 50); // 50ms aralıklarla
    }
    return NULL;
}

// Tüm drone ve survivor durumlarını içeren JSON mesajını oluşturur
static char *create_status_json() {
    json_object *jobj = json_object_new_object();
    json_object *jdrones = json_object_new_array();
    json_object *jsurvivors = json_object_new_array();
    
    json_object_object_add(jobj, "type", json_object_new_string("status_update"));
    
    // Drone'ları ekle
    if (drones) {
        pthread_mutex_lock(&drones->lock);
        Node *current = drones->head;
        while (current != NULL) {
            Drone **drone_ptr = (Drone **)current->data;
            if (drone_ptr && *drone_ptr) {
                Drone *drone = *drone_ptr;
                
                json_object *jdrone = json_object_new_object();
                json_object_object_add(jdrone, "id", json_object_new_string(drone->id));
                
                // DroneStatus'u string'e dönüştür
                const char *status_str = "unknown";
                switch(drone->status) {
                    case IDLE:
                        status_str = "idle";
                        break;
                    case ON_MISSION:
                        status_str = "busy";
                        break;
                    case CHARGING:
                        status_str = "charging";
                        break;
                }
                json_object_object_add(jdrone, "status", json_object_new_string(status_str));
                
                json_object *jpos = json_object_new_object();
                json_object_object_add(jpos, "x", json_object_new_int(drone->coord.x));
                json_object_object_add(jpos, "y", json_object_new_int(drone->coord.y));
                json_object_object_add(jdrone, "position", jpos);
                
                json_object *jtarget = json_object_new_object();
                json_object_object_add(jtarget, "x", json_object_new_int(drone->target.x));
                json_object_object_add(jtarget, "y", json_object_new_int(drone->target.y));
                json_object_object_add(jdrone, "target", jtarget);
                
                json_object_array_add(jdrones, jdrone);
            }
            current = current->next;
        }
        pthread_mutex_unlock(&drones->lock);
    }
    
    // survivorleri ekle
    if (survivors) {
        pthread_mutex_lock(&survivors->lock);
        Node *current = survivors->head;
        while (current != NULL) {
            Survivor *survivor = (Survivor *)current->data;
            if (survivor) {
                json_object *jsurvivor = json_object_new_object();
                // Survivor ID'si yok, konum bilgisini kullanarak bir ID oluştur
                char survivor_id[32];
                sprintf(survivor_id, "s_%d_%d", survivor->coord.x, survivor->coord.y);
                json_object_object_add(jsurvivor, "id", json_object_new_string(survivor_id));
                
                json_object *jpos = json_object_new_object();
                json_object_object_add(jpos, "x", json_object_new_int(survivor->coord.x));
                json_object_object_add(jpos, "y", json_object_new_int(survivor->coord.y));
                json_object_object_add(jsurvivor, "position", jpos);
                
                json_object_object_add(jsurvivor, "status", json_object_new_string("waiting"));
                json_object_array_add(jsurvivors, jsurvivor);
            }
            current = current->next;
        }
        pthread_mutex_unlock(&survivors->lock);
    }
    
    // Kurtarılmış survivorleri ekle
    if (helpedsurvivors) {
        pthread_mutex_lock(&helpedsurvivors->lock);
        Node *current = helpedsurvivors->head;
        while (current != NULL) {
            Survivor *survivor = (Survivor *)current->data;
            if (survivor) {
                json_object *jsurvivor = json_object_new_object();
                // Survivor ID'si yok, konum bilgisini kullanarak bir ID oluştur
                char survivor_id[32];
                sprintf(survivor_id, "s_%d_%d", survivor->coord.x, survivor->coord.y);
                json_object_object_add(jsurvivor, "id", json_object_new_string(survivor_id));
                
                json_object *jpos = json_object_new_object();
                json_object_object_add(jpos, "x", json_object_new_int(survivor->coord.x));
                json_object_object_add(jpos, "y", json_object_new_int(survivor->coord.y));
                json_object_object_add(jsurvivor, "position", jpos);
                
                json_object_object_add(jsurvivor, "status", json_object_new_string("helped"));
                json_object_array_add(jsurvivors, jsurvivor);
            }
            current = current->next;
        }
        pthread_mutex_unlock(&helpedsurvivors->lock);
    }
    
    json_object_object_add(jobj, "drones", jdrones);
    json_object_object_add(jobj, "survivors", jsurvivors);
    
    const char *json_str = json_object_to_json_string(jobj);
    char *result = strdup(json_str);
    
    json_object_put(jobj);
    return result;
}

// WebSocket sunucusunu başlatır
int start_websocket_server(int port) {
    if (ws_running) {
        printf("WebSocket sunucusu zaten çalışıyor\n");
        return 0;
    }
    
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    
    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "WebSocket sunucusu başlatılamadı\n");
        return -1;
    }
    
    ws_running = true;
    
    // WebSocket thread'ini başlat
    if (pthread_create(&ws_thread, NULL, websocket_thread, NULL) != 0) {
        perror("WebSocket thread oluşturulamadı");
        lws_context_destroy(context);
        ws_running = false;
        return -1;
    }
    
    printf("WebSocket sunucusu %d portunda başlatıldı\n", port);
    return 0;
}

// WebSocket sunucusunu durdurur
void stop_websocket_server() {
    if (!ws_running) return;
    
    ws_running = false;
    pthread_join(ws_thread, NULL);
    
    lws_context_destroy(context);
    printf("WebSocket sunucusu durduruldu\n");
}

// Tüm web istemcilerine JSON mesajı gönderir
void websocket_broadcast(const char *json_message) {
    if (!ws_running || !json_message) return;
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        send_to_client(clients[i], json_message, strlen(json_message));
    }
    pthread_mutex_unlock(&clients_mutex);
}

// WebSocket sunucusunun çalışıp çalışmadığını kontrol eder
bool is_websocket_running() {
    return ws_running;
}

// Yeni bir drone başlatma isteğini işler
int launch_drone_client(const char *drone_id, const char *server_ip, int server_port) {
    char port_str[10];
    sprintf(port_str, "%d", server_port);
    
    // Drone istemcisini çalıştır
    pid_t pid;
    char *args[] = {"./drone_client", (char *)drone_id, (char *)server_ip, port_str, NULL};
    
    int result = posix_spawn(&pid, "./drone_client", NULL, NULL, args, NULL);
    if (result != 0) {
        perror("Drone istemcisi başlatılamadı");
        return -1;
    }
    
    printf("Drone istemcisi başlatıldı: ID=%s, IP=%s, Port=%s (PID: %d)\n", 
           drone_id, server_ip, port_str, pid);
    
    return 0;
} 