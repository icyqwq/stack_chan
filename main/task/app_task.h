#pragma once

#include <M5Unified.h>
#include "declaration.h"

typedef struct
{
	union {
		int data;
	};
	uint8_t cmd;
} app_common_cmd_t;



enum
{
	EYE_MODE_NORMAL = 0,
	EYE_MODE_DIZZY,
	EYE_MODE_HIDE,
	EYE_MODE_PAUSE,
};

enum
{
	ACTION_DIZZY = 0,
};

enum
{
	LED_MODE_RMS = 0,
	LED_MODE_ROTATE
};

enum
{
	WIFI_CMD_SEND_FRAME = 0,
};

void app_led_task_init();
void app_led_task_feed(float rms);
void app_led_set_mode(uint8_t mode);

void app_face_tracker_init();

void app_touch_task_init();

void app_eye_stop();
void app_eye_start();
void app_eye_task_init();
esp_err_t eye_send_cmd(uint8_t cmd, int data = 0);

void app_action_task_init();
esp_err_t action_send_cmd(uint8_t cmd, int data = 0);

void _wifi_event(WiFiEvent_t event);
void app_wifi_task_init();
esp_err_t wifi_send_cmd(uint8_t cmd, int data = 0);
uint8_t *app_wifi_get_img_buffer();