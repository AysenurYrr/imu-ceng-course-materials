#include "headers/drone_manager.h"
#include "headers/network.h"
#include "headers/json_helper.h"
#include "headers/protocol.h"
#include "headers/globals.h"
#include "headers/survivor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <uuid/uuid.h>
#define DEBUG_PRINT 0
// Global değişkenler
static int server_socket = -1;
extern volatile int server_should_exit; 
static volatile int server_running = 0;
pthread_mutex_t drone_list_mutex = PTHREAD_MUTEX_INITIALIZER;
// Bağlı drone'lar listesi - sunucu ve AI tarafından paylaşılıyor
List *connected_drones = NULL;
#define SOCKET_POLL_TIMEOUT_MS  100

// Drone istemcisiyle ilişkili thread'in çalıştırdığı fonksiyon
static void *handle_drone_client(void *arg);

// ServerDrone'dan Drone'a dönüştürme fonksiyonu
static Drone* create_drone_from_server_drone(ServerDrone* server_drone) {
    if (DEBUG_PRINT)
    printf("DEBUG: create_drone_from_server_drone başladı (drone_id: %s)\n", server_drone->drone_id);
    
    Drone* drone = (Drone*)malloc(sizeof(Drone));
    if (!drone) {
        printf("DEBUG: Drone için bellek ayrılamadı!\n");
        return NULL;
    }
    
    // Drone bilgilerini kopyala
    strncpy(drone->id, server_drone->drone_id, sizeof(drone->id) - 1);
    drone->id[sizeof(drone->id) - 1] = '\0'; // Null terminatör ekle
    
    drone->coord = server_drone->location;
    drone->target = server_drone->target;
    
    if (strcmp(server_drone->status, DRONE_STATUS_IDLE) == 0) {
        drone->status = IDLE;
    } else if (strcmp(server_drone->status, DRONE_STATUS_BUSY) == 0) {
        drone->status = ON_MISSION;
    } else {
        drone->status = IDLE;
    }
    if (DEBUG_PRINT)
    printf("DEBUG: create_drone_from_server_drone tamamlandı - drone oluşturuldu (id: %s, konum: %d,%d)\n", 
           drone->id, drone->coord.x, drone->coord.y);
    return drone;
}

// Drone'u global drones listesine ekle
static void add_drone_to_global_list(ServerDrone* server_drone) {
    if (DEBUG_PRINT)
    printf("DEBUG: add_drone_to_global_list başladı (drone_id: %s)\n", server_drone->drone_id);
    
    if (!drones) {
        printf("HATA: drones listesi başlatılmamış!\n");
        return;
    }
    
    if (DEBUG_PRINT)
    printf("DEBUG: drones listesi mevcut, eleman sayısı: %d\n", drones->number_of_elements);
    
    // Önce mevcut drone'u ara
    Node* current = drones->head;
    int node_count = 0;
    while (current) {
        node_count++;
        Drone** d_ptr = (Drone**)current->data;
        if (d_ptr && *d_ptr) {
            if (DEBUG_PRINT)
            printf("DEBUG: Liste elemanı #%d inceleniyor (id: %s)\n", node_count, (*d_ptr)->id);
            
            if (strcmp((*d_ptr)->id, server_drone->drone_id) == 0) {
                // Drone zaten listede, güncelle
                Drone* drone = *d_ptr;
                if (DEBUG_PRINT)
                printf("DEBUG: Drone %s listede bulundu, güncelleniyor\n", drone->id);
                
                drone->coord = server_drone->location;
                drone->target = server_drone->target;
                
                if (strcmp(server_drone->status, DRONE_STATUS_IDLE) == 0) {
                    drone->status = IDLE;
                } else if (strcmp(server_drone->status, DRONE_STATUS_BUSY) == 0) {
                    drone->status = ON_MISSION;
                }
                if (DEBUG_PRINT)
                printf("DEBUG: Drone %s güncellendi (konum: %d,%d)\n", 
                       drone->id, drone->coord.x, drone->coord.y);
                return;
            }
        } else {
            if (DEBUG_PRINT)
            printf("DEBUG: Liste elemanı #%d geçersiz pointer içeriyor\n", node_count);
        }
        current = current->next;
    }
    if (DEBUG_PRINT)
    printf("DEBUG: Drone %s listede bulunamadı, yeni ekleniyor\n", server_drone->drone_id);
    
    // Drone listede değil, yeni ekle
    Drone* new_drone = create_drone_from_server_drone(server_drone);
    if (new_drone) {
        Drone* drone_ptr = new_drone;
        Node* d = drones->add(drones, &drone_ptr);
        if (d != NULL) {
            if (DEBUG_PRINT)
            printf("DEBUG: Drone %s global listeye başarıyla eklendi (konum: %d,%d)\n", 
                   new_drone->id, new_drone->coord.x, new_drone->coord.y);
            if (DEBUG_PRINT)
            printf("DEBUG: drones listesi güncel eleman sayısı: %d\n", drones->number_of_elements);
        } else {
            if (DEBUG_PRINT)
            printf("HATA: Drone %s listeye eklenemedi!\n",d->data);
            free(new_drone);
        }
    } else {
        printf("HATA: Drone oluşturulamadı!\n");
    }
}

int init_drone_manager(void) {
    // Bağlı drone'lar için liste oluştur
    connected_drones = create_list(sizeof(ServerDrone*), 100);
    if (!connected_drones) {
        perror("Failed to create drone list");
        return -1;
    }
    
    return 0;
}

void cleanup_drone_manager(void) {
    pthread_mutex_lock(&drone_list_mutex);
    
    // Tüm bağlı drone'ları kapat
    Node *current = connected_drones->head;
    while (current) {
        ServerDrone **drone_ptr = (ServerDrone **)current->data;
        ServerDrone *drone = *drone_ptr;
        
        // Thread'i sonlandır
        pthread_cancel(drone->thread_id);
        pthread_join(drone->thread_id, NULL);
        
        // Soketi kapat
        close_socket(drone->socket_fd);
        
        // Mutex'i temizle
        pthread_mutex_destroy(&drone->lock);
        
        // Belleği serbest bırak
        free(drone);
        
        current = current->next;
    }
    
    // Listeyi temizle
    connected_drones->destroy(connected_drones);
    connected_drones = NULL;
    
    pthread_mutex_unlock(&drone_list_mutex);
    
    // Sunucu soketini kapat
    if (server_socket != -1) {
        close_socket(server_socket);
        server_socket = -1;
    }
}

int start_drone_server(int port) {
    if (server_running) {
        fprintf(stderr, "Server is already running\n");
        return -1;
    }
    
    // Drone yöneticisini başlat
    if (init_drone_manager() != 0) {
        return -1;
    }
    
    // Sunucu soketini oluştur
    server_socket = create_server_socket(port);
    if (server_socket == -1) {
        cleanup_drone_manager();
        return -1;
    }
    
    server_running = 1;
    if (DEBUG_PRINT)
    printf("Drone server started on port %d\n", port);
    
    return 0;
}

void stop_drone_server(void) {
    server_running = 0;
    cleanup_drone_manager();
    if (DEBUG_PRINT)
    printf("Drone server stopped\n");
}

void handle_connections(void) {
    if (!server_running || server_socket == -1) {
        fprintf(stderr, "Server is not running\n");
        return;
    }
    
    // Soketin okuma için hazır olup olmadığını kontrol et
    int ready = wait_for_socket(server_socket,
                                SOCKET_POLL_TIMEOUT_MS,   /* 100 ms bekle */
                                true, false);
    if (server_should_exit)        /* bekleme sırasında bayrak 1 olduysa */
        return;

    if (ready <= 0) {
        return; // Zaman aşımı veya hata
    }
    
    // Yeni bağlantıyı kabul et
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    
    if (client_socket < 0) {
        perror("Accept failed");
        return;
    }
    
    // İstemci bilgilerini al
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    get_client_info(&client_addr, client_ip, sizeof(client_ip), &client_port);
    printf("New connection from %s:%d\n", client_ip, client_port);
    
    // Yeni drone oluştur
    ServerDrone *drone = malloc(sizeof(ServerDrone));
    if (!drone) {
        perror("Failed to allocate memory for drone");
        close_socket(client_socket);
        return;
    }
    
    // Drone yapısını başlat
    memset(drone, 0, sizeof(ServerDrone));
    drone->socket_fd = client_socket;
    drone->location.x = -1; // Henüz bilinmiyor
    drone->location.y = -1;
    drone->target.x = -1;
    drone->target.y = -1;
    strcpy(drone->status, DRONE_STATUS_IDLE);
    drone->last_heartbeat = time(NULL);
    drone->missed_heartbeats = 0;
    pthread_mutex_init(&drone->lock, NULL);
    
    // Drone işleyici thread'ini oluştur
    ServerDrone *drone_ptr = drone;
    if (pthread_create(&drone->thread_id, NULL, handle_drone_client, (void *)drone) != 0) {
        perror("Failed to create drone thread");
        pthread_mutex_destroy(&drone->lock);
        free(drone);
        close_socket(client_socket);
        return;
    }
    
    // Bağlı drone'lar listesine ekle
    pthread_mutex_lock(&drone_list_mutex);
    connected_drones->add(connected_drones, &drone_ptr);
    pthread_mutex_unlock(&drone_list_mutex);
}

// Server tarafında JSON mesajlarını yazdıran yardımcı fonksiyon
void print_server_json(const char* prefix, const char* json_str) {
    printf("\n##### %s #####\n", prefix);
    printf("%s\n", json_str);
    printf("#################\n\n");
}

static void *handle_drone_client(void *arg) {
    ServerDrone *drone = (ServerDrone *)arg;
    if (!drone) return NULL;  // NULL kontrolü ekle
    
    char buffer[DEFAULT_BUFFER_SIZE];
    int bytes_received;
    int handshake_completed = 0;
    
    // Soketin ayarlarını yapılandır
    set_keepalive(drone->socket_fd, true);
    set_socket_timeout(drone->socket_fd, 5); // 5 saniye timeout
    
    // İstemci mesajlarını işle
    while (server_running) {
        // Veri al
        bytes_received = receive_data(drone->socket_fd, buffer, sizeof(buffer));
        
        if (bytes_received <= 0) {
            // Bağlantı kapandı veya hata oluştu
            if (bytes_received == 0) {
                printf("Connection closed by peer for drone %s\n", drone->drone_id);
            } else {
                perror("Error receiving data");
            }
            break;  // Döngüden çık
        }
        
        // Alınan JSON mesajını yazdır
        if (DEBUG_PRINT) {
            print_server_json("SERVER - MESAJ ALINDI", buffer);
        }
        
        // JSON mesajını çözümle
        const char *msg_type = get_message_type(buffer);
        if (!msg_type) {
            printf("Invalid JSON message received\n");
            // Hata yanıtı gönder
            json_object *error = create_error_msg(ERR_INVALID_JSON, "Invalid JSON format");
            const char *error_str = json_object_to_json_string(error);
            if (DEBUG_PRINT) {
                print_server_json("SERVER - HATA GÖNDER", error_str);
            }
            send_data(drone->socket_fd, error_str, strlen(error_str));
            json_object_put(error);
            continue;
        }
        
        // Mesaj tipine göre işle
        if (strcmp(msg_type, MSG_HANDSHAKE) == 0 && !handshake_completed) {
            // HANDSHAKE mesajını işle
            char drone_id[32];
            
            if (parse_handshake_msg(buffer, drone_id) != 0) {
                json_object *error = create_error_msg(ERR_INVALID_JSON, "Invalid HANDSHAKE message");
                const char *error_str = json_object_to_json_string(error);
                if (DEBUG_PRINT) {
                    print_server_json("SERVER - HATA GÖNDER", error_str);
                }
                send_data(drone->socket_fd, error_str, strlen(error_str));
                json_object_put(error);
                continue;
            }
            
            // Drone bilgilerini güncelle
            pthread_mutex_lock(&drone->lock);
            strcpy(drone->drone_id, drone_id);
            
            // Oturum ID oluştur
            generate_session_id(drone->session_id, sizeof(drone->session_id));
            pthread_mutex_unlock(&drone->lock);
            
            // HANDSHAKE_ACK yanıtı gönder
            json_object *ack = create_handshake_ack_msg(
                drone->session_id,
                DEFAULT_STATUS_UPDATE_INTERVAL,
                DEFAULT_HEARTBEAT_INTERVAL
            );
            const char *ack_str = json_object_to_json_string(ack);
            if (DEBUG_PRINT) {
                print_server_json("SERVER - HANDSHAKE_ACK GÖNDER", ack_str);
            }
            send_data(drone->socket_fd, ack_str, strlen(ack_str));
            json_object_put(ack);
            
            handshake_completed = 1;
            printf("Handshake completed with drone %s (Session: %s)\n", drone_id, drone->session_id);
            
            // Drone'u global listeye ekle
            add_drone_to_global_list(drone);
            
        } else if (strcmp(msg_type, MSG_STATUS_UPDATE) == 0) {
            // STATUS_UPDATE mesajını işle
            char drone_id[32];
            Coord location;
            char status[16];
            
            if (parse_status_update_msg(buffer, drone_id, &location, status) != 0) {
                json_object *error = create_error_msg(ERR_INVALID_JSON, "Invalid STATUS_UPDATE message");
                const char *error_str = json_object_to_json_string(error);
                if (DEBUG_PRINT) {
                    print_server_json("SERVER - HATA GÖNDER", error_str);
                }
                send_data(drone->socket_fd, error_str, strlen(error_str));
                json_object_put(error);
                continue;
            }
            
            // Drone bilgilerini güncelle
            pthread_mutex_lock(&drone->lock);
            drone->location = location;
            strcpy(drone->status, status);
            pthread_mutex_unlock(&drone->lock);
            
            if(DEBUG_PRINT)
            printf("Status update from drone %s: location=(%d,%d), status=%s\n",
                  drone_id, location.x, location.y, status);
            
            // Drone'u global listeye ekle/güncelle
            add_drone_to_global_list(drone);
            
        } else if (strcmp(msg_type, MSG_MISSION_COMPLETE) == 0) {
            // MISSION_COMPLETE mesajını işle
            char drone_id[32];
            char mission_id[32];
            int success;
            char details[128];
            
            if (parse_mission_complete_msg(buffer, drone_id, mission_id, &success, details) != 0) {
                json_object *error = create_error_msg(ERR_INVALID_JSON, "Invalid MISSION_COMPLETE message");
                const char *error_str = json_object_to_json_string(error);
                if (DEBUG_PRINT) {
                    print_server_json("SERVER - HATA GÖNDER", error_str);
                }
                send_data(drone->socket_fd, error_str, strlen(error_str));
                json_object_put(error);
                continue;
            }
            
            // Drone durumunu güncelle
            pthread_mutex_lock(&drone->lock);
            strcpy(drone->status, DRONE_STATUS_IDLE);
            
            // Hedef koordinatları kaydet
            Coord target = drone->target;
            
            memset(drone->mission_id, 0, sizeof(drone->mission_id));
            drone->target.x = -1;
            drone->target.y = -1;
            pthread_mutex_unlock(&drone->lock);
            
            printf("Mission completed by drone %s: mission=%s, success=%d, details=%s, target=(%d,%d)\n",
                  drone_id, mission_id, success, details, target.x, target.y);
            
            // Drone'u global listeye ekle/güncelle
            add_drone_to_global_list(drone);
            
            // Survivor durumunu güncelle
            if (success) {
                if (DEBUG_PRINT) {
                    printf("Calling survivor_finished for target (%d,%d)\n", target.x, target.y);
                }
                survivor_finished(target.x, target.y);
            }
            
        } else if (strcmp(msg_type, MSG_HEARTBEAT_RESPONSE) == 0) {
            // HEARTBEAT_RESPONSE mesajını işle
            pthread_mutex_lock(&drone->lock);
            drone->last_heartbeat = time(NULL);
            drone->missed_heartbeats = 0;
            pthread_mutex_unlock(&drone->lock);
            
        } else if (strcmp(msg_type, MSG_DISCONNECT) == 0) {
            // DISCONNECT mesajını işle - drone_client tarafından gönderilir
            printf("Received DISCONNECT message from drone %s\n", drone->drone_id);
            
            // Drone'un bağlantı durumunu güncelle
            pthread_mutex_lock(&drone->lock);
            strcpy(drone->status, DRONE_STATUS_DISCONNECTED);
            pthread_mutex_unlock(&drone->lock);
            
            // Drone'u global drones listesinden sil
            // disconnect_drone fonksiyonu connected_drones listesinden drone'u siler 
            // ancak global drones listesinden silmez
            // O yüzden burada drones listesinden de kaldırmamız gerekiyor
            
            // Global drones listesinden bu ID'ye sahip drone'u bul
            if (drones != NULL) {
                Node* current = drones->head;
                while (current) {
                    Drone** d_ptr = (Drone**)current->data;
                    if (d_ptr && *d_ptr && strcmp((*d_ptr)->id, drone->drone_id) == 0) {
                        // Drone'u bulduk, listeden kaldır
                        drones->removenode(drones, current);
                        printf("Drone %s removed from global drones list\n", drone->drone_id);
                        break; // Node'u bulduk ve sildik, döngüden çıkalım
                    }
                    current = current->next;
                }
            }
            
            // Döngüden çık ve bağlantıyı sonlandır
            break;
            
        } else {
            // Tanınmayan veya beklenmeyen mesaj
            printf("Unexpected message type: %s\n", msg_type);
            json_object *error = create_error_msg(ERR_INVALID_JSON, "Unexpected message type");
            const char *error_str = json_object_to_json_string(error);
            if (DEBUG_PRINT) {
                print_server_json("SERVER - HATA GÖNDER", error_str);
            }
            send_data(drone->socket_fd, error_str, strlen(error_str));
            json_object_put(error);
        }
        
        // Mesaj tipini serbest bırak
        free((void *)msg_type);
    }
    
    // Drone bağlantısını kapat
    printf("Closing connection for drone %s\n", drone->drone_id);
    disconnect_drone(drone);
    
    return NULL;
}

int assign_mission_to_drone(ServerDrone *drone, Coord target, const char *priority) {
    pthread_mutex_lock(&drone->lock);
    
    // Drone zaten bir görevde mi kontrol et
    if (strcmp(drone->status, DRONE_STATUS_IDLE) != 0) {
        pthread_mutex_unlock(&drone->lock);
        return -1;
    }
    
    // Görev ID'si oluştur
    generate_mission_id(drone->mission_id, sizeof(drone->mission_id));
    
    // Hedefi ayarla
    drone->target = target;
    
    // ASSIGN_MISSION mesajı oluştur
    json_object *mission = create_assign_mission_msg(
        drone->mission_id,
        priority,
        target
    );
    const char *mission_str = json_object_to_json_string(mission);
    
    // JSON mesajını yazdır
    if (DEBUG_PRINT) {
        print_server_json("SERVER - ASSIGN_MISSION GÖNDER", mission_str);
    }
    
    // Mesajı gönder
    int result = send_data(drone->socket_fd, mission_str, strlen(mission_str));
    json_object_put(mission);
    
    if (result > 0) {
        // Drone durumunu güncelle
        strcpy(drone->status, DRONE_STATUS_BUSY);
        printf("Mission assigned to drone %s: target=(%d,%d), priority=%s, mission_id=%s\n",
               drone->drone_id, target.x, target.y, priority, drone->mission_id);
        
        // Drone'u global listeye ekle/güncelle
        add_drone_to_global_list(drone);
    }
    
    pthread_mutex_unlock(&drone->lock);
    
    return (result > 0) ? 0 : -1;
}

int send_heartbeats_to_all(void) {
    int success_count = 0;
    
    pthread_mutex_lock(&drone_list_mutex);
    
    // Heartbeat mesajı oluştur
    json_object *heartbeat = create_heartbeat_msg();
    const char *heartbeat_str = json_object_to_json_string(heartbeat);
    
    // JSON mesajını yazdır (sadece bir kez yazdıralım)
    if (connected_drones->head != NULL && DEBUG_PRINT) {
        print_server_json("SERVER - HEARTBEAT GÖNDER", heartbeat_str);
    }
    
    // Tüm bağlı drone'lara heartbeat gönder
    Node *current = connected_drones->head;
    while (current) {
        ServerDrone **drone_ptr = (ServerDrone **)current->data;
        ServerDrone *drone = *drone_ptr;
        
        pthread_mutex_lock(&drone->lock);
        
        // Son heartbeat'ten bu yana geçen süreyi kontrol et
        time_t now = time(NULL);
        if (now - drone->last_heartbeat > DEFAULT_HEARTBEAT_INTERVAL * 2) {
            // Drone yanıt vermiyor
            drone->missed_heartbeats++;
            
            if (drone->missed_heartbeats >= MAX_MISSED_HEARTBEATS) {
                // Drone bağlantısını kaybolmuş olarak işaretle
                strcpy(drone->status, DRONE_STATUS_DISCONNECTED);
                printf("Drone %s marked as disconnected (missed %d heartbeats)\n",
                       drone->drone_id, drone->missed_heartbeats);
            }
        }
        
        // Heartbeat gönder
        if (strcmp(drone->status, DRONE_STATUS_DISCONNECTED) != 0) {
            if (send_data(drone->socket_fd, heartbeat_str, strlen(heartbeat_str)) > 0) {
                success_count++;
            }
        }
        
        pthread_mutex_unlock(&drone->lock);
        current = current->next;
    }
    
    json_object_put(heartbeat);
    pthread_mutex_unlock(&drone_list_mutex);
    
    return success_count;
}

void disconnect_drone(ServerDrone *drone) {
    if (!drone) return;  // NULL kontrolü ekle
    
    printf("Drone %s disconnecting...\n", drone->drone_id);
    
    // Drone'u connected_drones listesinden kaldır
    pthread_mutex_lock(&drone_list_mutex);
    
    Node *current = connected_drones->head;
    Node *to_remove = NULL;
    
    while (current) {
        ServerDrone **drone_ptr = (ServerDrone **)current->data;
        if (*drone_ptr == drone) {
            to_remove = current;
            break;
        }
        current = current->next;
    }
    
    if (to_remove) {
        connected_drones->removenode(connected_drones, to_remove);
        printf("Drone %s removed from connected drones list\n", drone->drone_id);
    }
    
    pthread_mutex_unlock(&drone_list_mutex);
    
    // Ayrıca drone'u global drones listesinden de kaldır
    if (drones != NULL) {
        Node* current = drones->head;
        while (current) {
            Drone** d_ptr = (Drone**)current->data;
            if (d_ptr && *d_ptr && strcmp((*d_ptr)->id, drone->drone_id) == 0) {
                // Drone'u bulduk, listeden kaldır
                drones->removenode(drones, current);
                printf("Drone %s removed from global drones list\n", drone->drone_id);
                break; // Node'u bulduk ve sildik, döngüden çıkalım
            }
            current = current->next;
        }
    }
    
    // Soketi kapat
    if (drone->socket_fd >= 0) {
        close_socket(drone->socket_fd);
        drone->socket_fd = -1;
    }
    
    // Mutex'i yok et
    pthread_mutex_destroy(&drone->lock);
    
    // Belleği serbest bırak
    free(drone);
}

void print_drone_status(void) {
    pthread_mutex_lock(&drone_list_mutex);
    
    printf("\n--- Connected Drones (%d) ---\n", connected_drones->number_of_elements);
    
    Node *current = connected_drones->head;
    while (current) {
        ServerDrone **drone_ptr = (ServerDrone **)current->data;
        ServerDrone *drone = *drone_ptr;
        
        pthread_mutex_lock(&drone->lock);
        
        printf("Drone ID: %s\n", drone->drone_id);
        printf("  Session: %s\n", drone->session_id);
        printf("  Status: %s\n", drone->status);
        printf("  Location: (%d,%d)\n", drone->location.x, drone->location.y);
        
        if (strcmp(drone->status, DRONE_STATUS_BUSY) == 0) {
            printf("  Mission: %s\n", drone->mission_id);
            printf("  Target: (%d,%d)\n", drone->target.x, drone->target.y);
        }
        
        printf("  Last heartbeat: %ld sec ago\n", time(NULL) - drone->last_heartbeat);
        printf("\n");
        
        pthread_mutex_unlock(&drone->lock);
        current = current->next;
    }
    
    pthread_mutex_unlock(&drone_list_mutex);
}

void generate_session_id(char *buffer, size_t size) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, buffer);
}

void generate_mission_id(char *buffer, size_t size) {
    snprintf(buffer, size, "M%ld", time(NULL) % 10000);
} 