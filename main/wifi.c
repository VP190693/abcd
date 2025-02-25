/*
 * wifi.c
 *
 *  Created on: 24-Feb-2025
 *      Author: Lenovo
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "tasks_common.h"
#include "wifi.h"

static const char TAG[] = "wifi";

static QueueHandle_t wifi_queue_handle;

esp_netif_t *esp_netif_sta = NULL;

static void wifi_event_handler(void *arg,esp_event_base_t event_base, int32_t event_id,void *event_data)
{
	if(event_base == WIFI_EVENT)
	{
		switch(event_id)
		{
			case WIFI_EVENT_STA_START:
				ESP_LOGI(TAG,"WIFI_EVENT_STA_START");
				break;

			case WIFI_EVENT_STA_STOP:
				ESP_LOGI(TAG,"WIFI_EVENT_STA_STOP");
				break;

			case WIFI_EVENT_STA_CONNECTED:
				ESP_LOGI(TAG,"WIFI_EVENT_STA_CONNECTED");
				break;

			case WIFI_EVENT_STA_DISCONNECTED:
				ESP_LOGI(TAG,"WIFI_EVENT_STA_DISCONNECTED");
				break;
		}
	}
	else if(event_base == IP_EVENT)
	{
		switch(event_id)
		{
			case IP_EVENT_STA_GOT_IP:
				ESP_LOGI(TAG,"IP_EVENT_STA_GOT_IP");
				break;
		}
	}
}

static void wifi_event_handler_init(void)
{
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_event_handler_instance_t instance_wifi_event;
	esp_event_handler_instance_t instance_ip_event;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_wifi_event));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_ip_event));

}

static void wifi_default_wifi_init(void)
{
	ESP_ERROR_CHECK(esp_netif_init());
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	esp_netif_sta = esp_netif_create_default_wifi_sta();
}

static void wifi_sta_config(void)
{
	wifi_config_t sta_config=
	{
		.sta=
		{
				.ssid = WIFI_APP_STA_SSID,
				.password =WIFI_APP_STA_PASSWORD
		}
	};
	//not used ip display functions here, as it is optional
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
	ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_STA, WIFI_APP_STA_BW));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_APP_STA_POWER_SAVE));
}

//Main task for wifi
static void wifi_task(void *pvParameters)
{
	wifi_queue_message_t msg;

	wifi_event_handler_init();

	wifi_default_wifi_init();

	wifi_sta_config();

	ESP_ERROR_CHECK(esp_wifi_start());

	wifi_send_message(WIFI_MSG_START_HTTP_SERVER);

	for(;;)
	{
		if(xQueueReceive(wifi_queue_handle, &msg, portMAX_DELAY))
		{
			switch(msg.msgID)
			{
				case WIFI_MSG_START_HTTP_SERVER:
					ESP_LOGI(TAG,"WIFI_MSG_START_HTTP_SERVER");
				//	http_server_start();
					break;

				case WIFI_MSG_CONNECTING_FROM_HTTP_SERVER:
					ESP_LOGI(TAG,"WIFI_MSG_CONNECTING_FROM_HTTP_SERVER");
				    break;

				case WIFI_MSG_STA_CONNECTED_GOT_IP:
					ESP_LOGI(TAG,"WIFI_MSG_STA_CONNECTED_GOT_IP");
					break;

				default:
					break;
			}
		}
	}
}

BaseType_t wifi_send_message(wifi_message_e msgID)
{
	wifi_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(wifi_queue_handle, &msg, portMAX_DELAY);
}


void wifi_start(void)
{
     ESP_LOGI(TAG, "STARTING WIFI");

     //disable default wifi log messages
     esp_log_level_set("wifi",ESP_LOG_NONE);

     wifi_queue_handle = xQueueCreate(3, sizeof(wifi_queue_message_t));

     //start wifi task
     xTaskCreatePinnedToCore(&wifi_task, "wifi_task", WIFI_TASK_STACK_SIZE, NULL, WIFI_TASK_PRIORITY, NULL, WIFI_TASK_CORE_ID);
}



