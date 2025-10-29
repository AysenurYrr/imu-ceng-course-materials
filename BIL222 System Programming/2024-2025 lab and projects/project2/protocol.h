/**
 * @file protocol.h
 * @brief İletişim protokolü tanımlamaları
 * 
 * Drone ve sunucu arasındaki iletişim için gerekli mesaj tipleri,
 * formatları ve sabitleri tanımlar.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

/* Mesaj tipleri */
#define MSG_HANDSHAKE_ACK        "HANDSHAKE_ACK"       // Server > Drone
#define MSG_ASSIGN_MISSION       "ASSIGN_MISSION"      // Server > Drone
#define MSG_HEARTBEAT            "HEARTBEAT"           // Server > Drone

#define MSG_HANDSHAKE            "HANDSHAKE"           // Drone > Server
#define MSG_STATUS_UPDATE        "STATUS_UPDATE"       // Drone > Server
#define MSG_HEARTBEAT_RESPONSE   "HEARTBEAT_RESPONSE"  // Drone > Server
#define MSG_MISSION_COMPLETE     "MISSION_COMPLETE"    // Drone > Server
#define MSG_DISCONNECT           "DISCONNECT"          // Drone > Server
#define MSG_ERROR                "ERROR"               // Server > Drone, Drone > Server

/* Drone durumları */
#define DRONE_STATUS_IDLE        "idle"
#define DRONE_STATUS_BUSY        "busy"
#define DRONE_STATUS_DISCONNECTED "disconnected"

/* Görev Öncelikleri */
#define PRIORITY_LOW            "low"
#define PRIORITY_MEDIUM         "medium"
#define PRIORITY_HIGH           "high"

/* Hata kodları */
#define ERR_INVALID_JSON         100
#define ERR_INVALID_SESSION      101
#define ERR_MISSION_FAILED       102

/* Diğer sabitler */
#define DEFAULT_STATUS_UPDATE_INTERVAL  1  // seconds
#define DEFAULT_HEARTBEAT_INTERVAL      10 // seconds
#define MAX_MISSED_HEARTBEATS           3  // Kaç heartbeat kaçırıldığında drone'u disconnected olarak işaretleyelim

/* Ağ Yapılandırması */
#define DEFAULT_SERVER_PORT    8080
#define DEFAULT_BUFFER_SIZE    1024

#endif /* PROTOCOL_H */ 