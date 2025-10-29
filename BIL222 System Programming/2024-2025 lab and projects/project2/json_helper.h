/**
 * @file json_helper.h
 * @brief JSON mesajlarını işlemek için yardımcı fonksiyonlar
 */
#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <json-c/json.h>
#include "drone.h"
#include "survivor.h"
#include "protocol.h"

/* Drone -> Sunucu mesajları */

/**
 * @brief HANDSHAKE mesajı oluşturur
 * @param drone_id Drone ID
 * @return json_object* Oluşturulan JSON mesajı (kullanım sonrası json_object_put çağrılmalıdır)
 */
json_object* create_handshake_msg(const char* drone_id);

/**
 * @brief STATUS_UPDATE mesajı oluşturur
 * @param drone_id Drone ID
 * @param location Konum
 * @param status Durum (idle, busy)
 * @return json_object* Oluşturulan JSON mesajı
 */
json_object* create_status_update_msg(const char* drone_id, Coord location, const char* status);

/**
 * @brief MISSION_COMPLETE mesajı oluşturur
 * @param drone_id Drone ID
 * @param mission_id Görev ID
 * @param success Başarı durumu
 * @param details Detaylar
 * @return json_object* Oluşturulan JSON mesajı
 */
json_object* create_mission_complete_msg(const char* drone_id, const char* mission_id, int success, const char* details);

/**
 * @brief HEARTBEAT_RESPONSE mesajı oluşturur
 * @param drone_id Drone ID
 * @return json_object* Oluşturulan JSON mesajı
 */
json_object* create_heartbeat_response_msg(const char* drone_id);

/* Sunucu -> Drone mesajları */

/**
 * @brief HANDSHAKE_ACK mesajı oluşturur
 * @param session_id Oturum ID
 * @param status_update_interval Durum güncelleme aralığı (saniye)
 * @param heartbeat_interval Heartbeat aralığı (saniye)
 * @return json_object* Oluşturulan JSON mesajı
 */
json_object* create_handshake_ack_msg(const char* session_id, int status_update_interval, int heartbeat_interval);

/**
 * @brief ASSIGN_MISSION mesajı oluşturur
 * @param mission_id Görev ID
 * @param priority Öncelik
 * @param target Hedef koordinatlar
 * @return json_object* Oluşturulan JSON mesajı
 */
json_object* create_assign_mission_msg(const char* mission_id, const char* priority, Coord target);

/**
 * @brief HEARTBEAT mesajı oluşturur
 * @return json_object* Oluşturulan JSON mesajı
 */
json_object* create_heartbeat_msg(void);

/**
 * @brief ERROR mesajı oluşturur
 * @param code Hata kodu
 * @param message Hata mesajı
 * @return json_object* Oluşturulan JSON mesajı
 */
json_object* create_error_msg(int code, const char* message);

/* Mesaj işleme fonksiyonları */

/**
 * @brief JSON mesajını ayrıştırır ve tipini döndürür
 * @param json_str JSON mesajı
 * @return const char* Mesaj tipi (protocol.h'da tanımlı MSG_ değerleri)
 */
const char* get_message_type(const char* json_str);

/**
 * @brief HANDSHAKE mesajını ayrıştırır
 * @param json_str JSON mesajı
 * @param drone_id Çıktı: Drone ID
 * @return int 0: başarılı, -1: hata
 */
int parse_handshake_msg(const char* json_str, char* drone_id);

/**
 * @brief STATUS_UPDATE mesajını ayrıştırır
 * @param json_str JSON mesajı
 * @param drone_id Çıktı: Drone ID
 * @param location Çıktı: Konum
 * @param status Çıktı: Durum
 * @return int 0: başarılı, -1: hata
 */
int parse_status_update_msg(const char* json_str, char* drone_id, Coord* location, char* status);

/**
 * @brief MISSION_COMPLETE mesajını ayrıştırır
 * @param json_str JSON mesajı
 * @param drone_id Çıktı: Drone ID
 * @param mission_id Çıktı: Görev ID
 * @param success Çıktı: Başarı durumu
 * @param details Çıktı: Detaylar
 * @return int 0: başarılı, -1: hata
 */
int parse_mission_complete_msg(const char* json_str, char* drone_id, char* mission_id, int* success, char* details);

/**
 * @brief ASSIGN_MISSION mesajını ayrıştırır
 * @param json_str JSON mesajı
 * @param mission_id Çıktı: Görev ID
 * @param priority Çıktı: Öncelik
 * @param target Çıktı: Hedef koordinatlar
 * @return int 0: başarılı, -1: hata
 */
int parse_assign_mission_msg(const char* json_str, char* mission_id, char* priority, Coord* target);

#endif /* JSON_HELPER_H */ 