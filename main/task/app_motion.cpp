#include <Arduino.h>
#include <M5Unified.h>
#include "app_motion.h"
#include "declaration.h"

#define TAG "Motion"
static QueueHandle_t queue_motion_cmd = NULL;
static bool action_running = false;

void motion_nod(int times)
{
	int last_pos = servo_2.getPosition();
	for (int i = 0; i < times; i++) {
		servo_2.setPosition(servo_2.getMin() + 50);
		vTaskDelay(100);
		servo_2.setPosition(servo_2.getMax() - 50);
		vTaskDelay(100);
	}
	servo_2.setPosition(last_pos);
}

void motion_dizzy(int times)
{
	servo_2.backToDefault();
	servo_1.backToDefault();

	int servo1_default = servo_1.getDefaultPosition();
	for (int i = 0; i < times; i++) {
		servo_1.setPosition(random(servo1_default - 200, servo1_default + 200));
		servo_2.setPosition(random(servo_2.getMin(), servo_2.getMax()));
		vTaskDelay(100);
	}

	servo_2.backToDefault();
	servo_1.backToDefault();
}

void motion_task(void *args)
{
	app_common_cmd_t motion;
	uint32_t next_wait_time = portMAX_DELAY;
	bool need_back = false;
	bool is_motion_exec = false;
	bool is_talking = false;
	while (1)
	{
		if (xQueueReceive(queue_motion_cmd, &motion, next_wait_time) != pdTRUE) {
			if (need_back) {
				servo_2.backToDefault();
				servo_1.backToDefault();
			}
			if (!is_motion_exec) {
				next_wait_time = portMAX_DELAY;
				continue;
			}
			if (is_talking) {
				servo_2.setPosition(random(servo_2.getMin(), servo_2.getMax()));
				continue;
			}
		}
		else {
			ESP_LOGI(TAG, "CMD: %d, data: %d", motion.cmd, motion.data);
		}


		switch (motion.cmd)
		{
		case MOTION_CMD_NOD:
		{
			action_running = true;
			motion_nod(motion.data);
			action_running = false;
			break;	
		}
		case MOTION_CMD_TURN_RIGHT:
		{
			servo_1.setPosition(servo_1.getPosition(UNIT_DEGREE) + motion.data, UNIT_DEGREE);
			need_back = true;
			next_wait_time = 5000;
			break;	
		}
		case MOTION_CMD_TURN_LEFT:
		{
			servo_1.setPosition(servo_1.getPosition(UNIT_DEGREE) - motion.data, UNIT_DEGREE);
			need_back = true;
			next_wait_time = 5000;
			break;	
		}
		case MOTION_CMD_BACK:
		{
			servo_2.backToDefault();
			servo_1.backToDefault();
			break;	
		}

		case MOTION_CMD_TALK_START:
		{
			servo_2.setPosition(random(servo_2.getMin(), servo_2.getMax()));
			next_wait_time = 100;
			is_talking = true;
			is_motion_exec = true;
			break;
		}

		case MOTION_CMD_TALK_END:
		{
			need_back = true;
			is_talking = false;
			is_motion_exec = false;
			break;
		}

		case MOTION_CMD_DIZZY:
		{
			action_running = true;
			motion_dizzy(motion.data);
			action_running = false;
			break;
		}
		
		default:
			break;
		}
	}
	
}

void motion_init()
{
	queue_motion_cmd = xQueueCreate(8, sizeof(app_common_cmd_t));
	xTaskCreatePinnedToCore(&motion_task, "Motion", 8 * 1024, NULL, 15, NULL, 1);

}

esp_err_t motion_send_cmd(uint8_t cmd, int data)
{
	if (action_running) {
		return ESP_FAIL;
	}
	if (!queue_motion_cmd) {
		return ESP_FAIL;
	}

	app_common_cmd_t motion;
	motion.cmd = cmd;
	motion.data = data;
	if (xQueueSend(queue_motion_cmd, &motion, 0) != pdTRUE) {
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t motion_send_cmd(uint8_t cmd)
{
	if (action_running) {
		return ESP_FAIL;
	}
	if (!queue_motion_cmd) {
		return ESP_FAIL;
	}

	app_common_cmd_t motion;
	motion.cmd = cmd;
	if (xQueueSend(queue_motion_cmd, &motion, 0) != pdTRUE) {
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t motion_send_cmd(app_common_cmd_t motion)
{
	if (action_running) {
		return ESP_FAIL;
	}
	if (!queue_motion_cmd) {
		return ESP_FAIL;
	}

	if (xQueueSend(queue_motion_cmd, &motion, 0) != pdTRUE) {
		return ESP_FAIL;
	}
	return ESP_OK;
}
