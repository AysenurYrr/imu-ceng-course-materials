#include "headers/drone_manager.h"
#include "headers/server_ai.h"
#include "headers/survivor.h"
#include "headers/globals.h"
#include "headers/protocol.h"
#include "headers/map.h"
#include "headers/view.h"
#include "headers/websocket_server.h"
#include "headers/http_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <json-c/json.h>

// Global değişkenler
volatile int server_should_exit = 0;
pthread_t survivor_generator_thread;
pthread_t survivor_cleanup_thread;
pthread_t view_thread;
pthread_t status_broadcast_thread;

// Bağlı drone'lar listesine erişim için extern
extern List *connected_drones;

// WebSocket için varsayılan port
#define DEFAULT_WEBSOCKET_PORT 8081
// HTTP için varsayılan port
#define DEFAULT_HTTP_PORT 8082

// Sinyal işleyici
void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nReceived SIGINT, shutting down server...\n");
        server_should_exit = 1;
    }
}

// SDL görselleştirme thread'i
void *view_thread_func(void *arg) {
    printf("SDL görselleştirme başlatılıyor...\n");
    
    // SDL penceresini başlat
    if (init_sdl_window() != 0) {
        fprintf(stderr, "SDL penceresi başlatılamadı\n");
        return NULL;
    }
    
    // Ana döngü
    while (!server_should_exit) {
        // Olayları kontrol et
        if (check_events()) {
            server_should_exit = 1;
            break;
        }
        
        // Haritayı çiz
        draw_map();
        
        // Kısa bir süre bekle
        SDL_Delay(100);
    }
    
    // SDL'i temizle
    quit_all();
    printf("SDL görselleştirme durduruldu\n");
    return NULL;
}

// Durum güncelleme thread'i - WebSocket istemcilerine durum güncellemeleri gönderir
void *status_broadcast_func(void *arg) {
    printf("WebSocket durum yayını başlatılıyor...\n");
    
    while (!server_should_exit) {
        if (is_websocket_running()) {
            // Durum JSON'ını oluştur ve yayınla
            json_object *jobj = json_object_new_object();
            json_object *jdrones = json_object_new_array();
            json_object *jsurvivors = json_object_new_array();
            
            json_object_object_add(jobj, "type", json_object_new_string("status_update"));
            json_object_object_add(jobj, "timestamp", json_object_new_int64(time(NULL)));
            
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
            
            // Kurtarılmış kazazedeleri ekle
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
            websocket_broadcast(json_str);
            
            json_object_put(jobj);
        }
        
        // Her 1 saniyede bir güncelle
        sleep(1);
    }
    
    printf("WebSocket durum yayını durduruldu\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    // Port numarasını ayarla (varsayılan: 8080)
    int port = DEFAULT_SERVER_PORT;
    int ws_port = DEFAULT_WEBSOCKET_PORT;
    int http_port = DEFAULT_HTTP_PORT;
    
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default: %d\n", DEFAULT_SERVER_PORT);
            port = DEFAULT_SERVER_PORT;
        }
    }
    
    if (argc > 2) {
        ws_port = atoi(argv[2]);
        if (ws_port <= 0 || ws_port > 65535 || ws_port == port) {
            fprintf(stderr, "Invalid WebSocket port number or same as server port. Using default: %d\n", DEFAULT_WEBSOCKET_PORT);
            ws_port = DEFAULT_WEBSOCKET_PORT;
        }
    }
    
    if (argc > 3) {
        http_port = atoi(argv[3]);
        if (http_port <= 0 || http_port > 65535 || http_port == port || http_port == ws_port) {
            fprintf(stderr, "Invalid HTTP port number or same as other ports. Using default: %d\n", DEFAULT_HTTP_PORT);
            http_port = DEFAULT_HTTP_PORT;
        }
    }
    
    // Sinyal işleyiciyi ayarla
    signal(SIGINT, signal_handler);
    
    // Global listeleri oluştur
    survivors = create_list(sizeof(Survivor), 1000);
    helpedsurvivors = create_list(sizeof(Survivor), 1000);
    drones = create_list(sizeof(Drone*), 100);
    
    // Haritayı başlat
    init_map(MAP_HEIGHT, MAP_WIDTH);
    
    // Survivor generator thread'ini başlat
    if (pthread_create(&survivor_generator_thread, NULL, survivor_generator, NULL) != 0) {
        perror("Failed to create survivor generator thread");
        exit(EXIT_FAILURE);
    }
    
    // Survivor cleanup thread'ini başlat
    if (pthread_create(&survivor_cleanup_thread, NULL, survivor_cleanup, NULL) != 0) {
        perror("Failed to create survivor cleanup thread");
        exit(EXIT_FAILURE);
    }
    
    // SDL görselleştirme thread'ini başlat
    if (pthread_create(&view_thread, NULL, view_thread_func, NULL) != 0) {
        perror("Failed to create view thread");
        exit(EXIT_FAILURE);
    }
    
    // HTTP sunucusunu başlat
    if (start_http_server(http_port, "./web") != 0) {
        fprintf(stderr, "HTTP sunucusu başlatılamadı\n");
        // HTTP olmadan devam et
    }
    
    // WebSocket sunucusunu başlat
    if (start_websocket_server(ws_port) != 0) {
        fprintf(stderr, "WebSocket sunucusu başlatılamadı\n");
        // WebSocket olmadan devam et
    } else {
        // WebSocket durum yayını thread'ini başlat
        if (pthread_create(&status_broadcast_thread, NULL, status_broadcast_func, NULL) != 0) {
            perror("Failed to create status broadcast thread");
            stop_websocket_server();
            // WebSocket durum yayını olmadan devam et
        }
    }
    
    // Drone sunucusunu başlat
    printf("Starting drone server on port %d...\n", port);
    if (start_drone_server(port) != 0) {
        fprintf(stderr, "Failed to start drone server\n");
        exit(EXIT_FAILURE);
    }
    
    // AI kontrolcüsünü başlat
    if (start_server_ai(connected_drones) != 0) {
        fprintf(stderr, "Failed to start AI controller\n");
        stop_drone_server();
        exit(EXIT_FAILURE);
    }
    
    printf("Server running. Press Ctrl+C to stop.\n");
    printf("WebSocket server running on port %d for web monitoring.\n", ws_port);
    printf("HTTP server running on port %d for web interface.\n", http_port);
    printf("Open http://localhost:%d in your browser to access the web interface.\n", http_port);
    
    // Ana döngü
    while (!server_should_exit) {
        // Gelen bağlantıları işle
        handle_connections();
        
        // Kısa bir süre bekle
        usleep(50000); // 50ms
    }
    
    printf("Shutting down...\n");
    
    // Kaynakları temizle
    stop_server_ai();
    stop_drone_server();
    stop_websocket_server();
    stop_http_server();
    
    // Thread'leri sonlandır
    pthread_cancel(survivor_generator_thread);
    pthread_join(survivor_generator_thread, NULL);
    
    pthread_cancel(survivor_cleanup_thread);
    pthread_join(survivor_cleanup_thread, NULL);
    
    pthread_cancel(view_thread);
    pthread_join(view_thread, NULL);
    
    if (is_websocket_running()) {
        pthread_cancel(status_broadcast_thread);
        pthread_join(status_broadcast_thread, NULL);
    }
    
    
    // Global listeleri temizle
    survivors->destroy(survivors);
    helpedsurvivors->destroy(helpedsurvivors);
    if (drones) drones->destroy(drones);
    
    printf("Server shutdown complete\n");
    
    return 0;
}
