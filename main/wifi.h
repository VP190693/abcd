/*
 * wifi.h
 *
 *  Created on: 24-Feb-2025
 *      Author: Lenovo
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include "esp_netif.h"
//#include "portmacro.h"
#include "freertos/FreeRTOS.h"
#define WIFI_APP_STA_SSID            "ESP32_STA"
#define WIFI_APP_STA_PASSWORD        "password"
#define WIFI_APP_STA_CHANNEL          0
#define WIFI_APP_STA_BW               WIFI_BW_HT20     //optional bandwidth definition
#define WIFI_APP_STA_POWER_SAVE       WIFI_PS_NONE
#define MAX_SSID_LENGTH           32
#define MAX_PASSWORD_LENGTH       64
#define MAX_CONNECTION_RETRIES    5

extern esp_netif_t *esp_netif_sta;

//Message IDs for the wifi task
typedef enum wifi_message
{
	WIFI_MSG_START_HTTP_SERVER =0,
	WIFI_MSG_CONNECTING_FROM_HTTP_SERVER,
	WIFI_MSG_STA_CONNECTED_GOT_IP,
} wifi_message_e;

//Structure for the message queue
typedef struct wifi_queue_message
{
	wifi_message_e msgID;
} wifi_queue_message_t;

//send a message to the queue
BaseType_t wifi_send_message(wifi_message_e msgID);

//starts the wifi rtos task
void wifi_start(void);

#endif /* MAIN_WIFI_H_ */
