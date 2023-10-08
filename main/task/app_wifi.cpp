#include "app_task.h"
#include "app_common.h"
#include "anime.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "esp_wifi.h"

#define TAG "WiFi"

WiFiUDP udp;
static QueueHandle_t queue_wifi_cmd = NULL;
static bool printer_connected = false;
IPAddress printer_ip;
uint8_t *_img_buffer = nullptr;

void _wifi_event(WiFiEvent_t event)
{
	wifi_sta_list_t wifi_sta_list;
	tcpip_adapter_sta_list_t adapter_sta_list;
	memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
	memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

	switch (event)
	{
	case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
		esp_wifi_ap_get_sta_list(&wifi_sta_list);
		tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

		for (int i = 0; i < adapter_sta_list.num; i++)
		{
			tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];

			printf("STA %d, IP %s\n", i, ip4addr_ntoa((const ip4_addr_t*)&(station.ip)));
		}
		printer_ip = IPAddress(adapter_sta_list.sta[0].ip.addr);
		vTaskDelay(1000);
		printer_connected = true;
		
		break;

	case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
		ESP_LOGW(TAG, "Printer lost connection");
		udp.stop();
		printer_connected = false;
		break;
	default:
		break;
	}
}

void app_wifi_task(void *args)
{
	app_common_cmd_t ctrl;
	
	while (1)
	{
		if (xQueueReceive(queue_wifi_cmd, &ctrl, portMAX_DELAY) != pdTRUE) {
			vTaskDelay(50);
			continue;
		}

		if (!printer_connected) {
			ESP_LOGW(TAG, "Printer not connected");
			vTaskDelay(50);
			continue;
		}

		switch (ctrl.cmd)
		{
		case WIFI_CMD_SEND_FRAME: {
			
			if(udp.begin(IPAddress(192, 168, 4, 1), 1234)) {
				ESP_LOGI(TAG, "UDP Connected");
			} else {
				ESP_LOGW(TAG, "UDP Connect failed");
				break;
			}
			uint8_t retry = 0;
			for (int i = 0; i < 8; i++) {
				udp.beginPacket(printer_ip, 1234);
				udp.write(i);
				int len = udp.write(_img_buffer + i * 1200, 1200);
				udp.endPacket();
				// int len = udp.write(_img_buffer + i * 1200, 1200);
				if (len != 1200) {
					ESP_LOGE(TAG, "Data transfer error, len %d != %d, pack = %d", len, 1200, i);
					retry++;
					if (retry > 3) {
						udp.stop();
						break;
					} else {
						i--;
					}
				} else {
					ESP_LOGI(TAG, "Pack %d ok", i);
				}
				vTaskDelay(50);
			}
			udp.stop();
			break;
		}
		
		default:
			break;
		}

		vTaskDelay(50);
	}
}

uint8_t *app_wifi_get_img_buffer()
{
	return _img_buffer;
}

void app_wifi_task_init()
{
	_img_buffer = (uint8_t*)heap_caps_malloc((320 * 240) / 8, MALLOC_CAP_SPIRAM);
	queue_wifi_cmd = xQueueCreate(8, sizeof(app_common_cmd_t));
	xTaskCreatePinnedToCore(&app_wifi_task, "WiFiTask", 8 * 1024, NULL, 8, NULL, 1);
}

esp_err_t wifi_send_cmd(uint8_t cmd, int data)
{
	if (!queue_wifi_cmd)
	{
		return ESP_FAIL;
	}

	app_common_cmd_t motion;
	motion.cmd = cmd;
	motion.data = data;
	if (xQueueSend(queue_wifi_cmd, &motion, 0) != pdTRUE)
	{
		return ESP_FAIL;
	}
	return ESP_OK;
}