#include "headers/json_helper.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* Drone -> Sunucu mesajları */

json_object* create_handshake_msg(const char* drone_id) {
    json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_HANDSHAKE));
    json_object_object_add(jobj, "drone_id", json_object_new_string(drone_id));
    
    return jobj;
}

json_object* create_status_update_msg(const char* drone_id, Coord location, const char* status) {
    json_object *jobj = json_object_new_object();
    json_object *loc = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_STATUS_UPDATE));
    json_object_object_add(jobj, "drone_id", json_object_new_string(drone_id));
    json_object_object_add(jobj, "timestamp", json_object_new_int64(time(NULL)));
    
    json_object_object_add(loc, "x", json_object_new_int(location.x));
    json_object_object_add(loc, "y", json_object_new_int(location.y));
    json_object_object_add(jobj, "location", loc);
    
    json_object_object_add(jobj, "status", json_object_new_string(status));
    
    return jobj;
}

json_object* create_mission_complete_msg(const char* drone_id, const char* mission_id, int success, const char* details) {
    json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_MISSION_COMPLETE));
    json_object_object_add(jobj, "drone_id", json_object_new_string(drone_id));
    json_object_object_add(jobj, "mission_id", json_object_new_string(mission_id));
    json_object_object_add(jobj, "timestamp", json_object_new_int64(time(NULL)));
    json_object_object_add(jobj, "success", json_object_new_boolean(success));
    json_object_object_add(jobj, "details", json_object_new_string(details));
    
    return jobj;
}

json_object* create_heartbeat_response_msg(const char* drone_id) {
    json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_HEARTBEAT_RESPONSE));
    json_object_object_add(jobj, "drone_id", json_object_new_string(drone_id));
    json_object_object_add(jobj, "timestamp", json_object_new_int64(time(NULL)));
    
    return jobj;
}

/* Sunucu -> Drone mesajları */

json_object* create_handshake_ack_msg(const char* session_id, int status_update_interval, int heartbeat_interval) {
    json_object *jobj = json_object_new_object();
    json_object *config = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_HANDSHAKE_ACK));
    json_object_object_add(jobj, "session_id", json_object_new_string(session_id));
    
    json_object_object_add(config, "status_update_interval", json_object_new_int(status_update_interval));
    json_object_object_add(config, "heartbeat_interval", json_object_new_int(heartbeat_interval));
    
    json_object_object_add(jobj, "config", config);
    
    return jobj;
}

json_object* create_assign_mission_msg(const char* mission_id, const char* priority, Coord target) {
    json_object *jobj = json_object_new_object();
    json_object *tgt = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_ASSIGN_MISSION));
    json_object_object_add(jobj, "mission_id", json_object_new_string(mission_id));
    json_object_object_add(jobj, "priority", json_object_new_string(priority));
    
    json_object_object_add(tgt, "x", json_object_new_int(target.x));
    json_object_object_add(tgt, "y", json_object_new_int(target.y));
    json_object_object_add(jobj, "target", tgt);
    
    // Opsiyonel checksum (basit örnek)
    char checksum[7] = {0};
    snprintf(checksum, sizeof(checksum), "%lx", (unsigned long)(target.x + target.y));
    json_object_object_add(jobj, "checksum", json_object_new_string(checksum));
    
    return jobj;
}

json_object* create_heartbeat_msg(void) {
    json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_HEARTBEAT));
    json_object_object_add(jobj, "timestamp", json_object_new_int64(time(NULL)));
    
    return jobj;
}

json_object* create_error_msg(int code, const char* message) {
    json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "type", json_object_new_string(MSG_ERROR));
    json_object_object_add(jobj, "code", json_object_new_int(code));
    json_object_object_add(jobj, "message", json_object_new_string(message));
    json_object_object_add(jobj, "timestamp", json_object_new_int64(time(NULL)));
    
    return jobj;
}

/* Mesaj işleme fonksiyonları */

const char* get_message_type(const char* json_str) {
    json_object *jobj = json_tokener_parse(json_str);
    if (!jobj) {
        return NULL; // Geçersiz JSON
    }
    
    json_object *type;
    if (!json_object_object_get_ex(jobj, "type", &type)) {
        json_object_put(jobj);
        return NULL; // "type" alanı bulunamadı
    }
    
    const char *type_str = json_object_get_string(type);
    const char *result = strdup(type_str); // Sonucu kopyala
    
    json_object_put(jobj);
    return result;
}

int parse_handshake_msg(const char* json_str, char* drone_id) {
    json_object *jobj = json_tokener_parse(json_str);
    if (!jobj) {
        return -1; // Geçersiz JSON
    }
    
    // Mesaj tipini kontrol et
    json_object *type;
    if (!json_object_object_get_ex(jobj, "type", &type) || 
        strcmp(json_object_get_string(type), MSG_HANDSHAKE) != 0) {
        json_object_put(jobj);
        return -1;
    }
    
    // Drone ID
    json_object *jdrone_id;
    if (!json_object_object_get_ex(jobj, "drone_id", &jdrone_id)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(drone_id, json_object_get_string(jdrone_id));
    
    json_object_put(jobj);
    return 0;
}

int parse_status_update_msg(const char* json_str, char* drone_id, Coord* location, char* status) {
    json_object *jobj = json_tokener_parse(json_str);
    if (!jobj) {
        return -1; // Geçersiz JSON
    }
    
    // Mesaj tipini kontrol et
    json_object *type;
    if (!json_object_object_get_ex(jobj, "type", &type) || 
        strcmp(json_object_get_string(type), MSG_STATUS_UPDATE) != 0) {
        json_object_put(jobj);
        return -1;
    }
    
    // Drone ID
    json_object *jdrone_id;
    if (!json_object_object_get_ex(jobj, "drone_id", &jdrone_id)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(drone_id, json_object_get_string(jdrone_id));
    
    // Location
    json_object *jlocation;
    if (!json_object_object_get_ex(jobj, "location", &jlocation)) {
        json_object_put(jobj);
        return -1;
    }
    
    json_object *jx, *jy;
    if (!json_object_object_get_ex(jlocation, "x", &jx) || 
        !json_object_object_get_ex(jlocation, "y", &jy)) {
        json_object_put(jobj);
        return -1;
    }
    
    location->x = json_object_get_int(jx);
    location->y = json_object_get_int(jy);
    
    // Status
    json_object *jstatus;
    if (!json_object_object_get_ex(jobj, "status", &jstatus)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(status, json_object_get_string(jstatus));
    
    json_object_put(jobj);
    return 0;
}

int parse_mission_complete_msg(const char* json_str, char* drone_id, char* mission_id, int* success, char* details) {
    json_object *jobj = json_tokener_parse(json_str);
    if (!jobj) {
        return -1; // Geçersiz JSON
    }
    
    // Mesaj tipini kontrol et
    json_object *type;
    if (!json_object_object_get_ex(jobj, "type", &type) || 
        strcmp(json_object_get_string(type), MSG_MISSION_COMPLETE) != 0) {
        json_object_put(jobj);
        return -1;
    }
    
    // Drone ID
    json_object *jdrone_id;
    if (!json_object_object_get_ex(jobj, "drone_id", &jdrone_id)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(drone_id, json_object_get_string(jdrone_id));
    
    // Mission ID
    json_object *jmission_id;
    if (!json_object_object_get_ex(jobj, "mission_id", &jmission_id)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(mission_id, json_object_get_string(jmission_id));
    
    // Success
    json_object *jsuccess;
    if (!json_object_object_get_ex(jobj, "success", &jsuccess)) {
        json_object_put(jobj);
        return -1;
    }
    *success = json_object_get_boolean(jsuccess);
    
    // Details
    json_object *jdetails;
    if (!json_object_object_get_ex(jobj, "details", &jdetails)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(details, json_object_get_string(jdetails));
    
    json_object_put(jobj);
    return 0;
}

int parse_assign_mission_msg(const char* json_str, char* mission_id, char* priority, Coord* target) {
    json_object *jobj = json_tokener_parse(json_str);
    if (!jobj) {
        return -1; // Geçersiz JSON
    }
    
    // Mesaj tipini kontrol et
    json_object *type;
    if (!json_object_object_get_ex(jobj, "type", &type) || 
        strcmp(json_object_get_string(type), MSG_ASSIGN_MISSION) != 0) {
        json_object_put(jobj);
        return -1;
    }
    
    // Mission ID
    json_object *jmission_id;
    if (!json_object_object_get_ex(jobj, "mission_id", &jmission_id)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(mission_id, json_object_get_string(jmission_id));
    
    // Priority
    json_object *jpriority;
    if (!json_object_object_get_ex(jobj, "priority", &jpriority)) {
        json_object_put(jobj);
        return -1;
    }
    strcpy(priority, json_object_get_string(jpriority));
    
    // Target
    json_object *jtarget;
    if (!json_object_object_get_ex(jobj, "target", &jtarget)) {
        json_object_put(jobj);
        return -1;
    }
    
    json_object *jx, *jy;
    if (!json_object_object_get_ex(jtarget, "x", &jx) || 
        !json_object_object_get_ex(jtarget, "y", &jy)) {
        json_object_put(jobj);
        return -1;
    }
    
    target->x = json_object_get_int(jx);
    target->y = json_object_get_int(jy);
    
    json_object_put(jobj);
    return 0;
} 