/**
 * @file server_ai.h
 * @brief Sunucu tarafı AI kontrolcüsü
 */
#ifndef SERVER_AI_H
#define SERVER_AI_H

#include "list.h"

/**
 * @brief Sunucu AI bileşenini başlat
 * @param drone_list Drone listesi
 * @return int 0: başarılı, -1: hata
 */
int start_server_ai(List *drone_list);

/**
 * @brief Sunucu AI bileşenini durdur
 */
void stop_server_ai(void);

#endif /* SERVER_AI_H */ 