#include "app_task.h"
#include "app_common.h"
#include "anime.h"
#include "app_motion.h"

#define TAG "Action"

static QueueHandle_t queue_action_cmd = NULL;

void app_action_task(void *args)
{
	app_common_cmd_t ctrl;

	vTaskDelay(5000);
	xQueueReset(queue_action_cmd);

	while (1)
	{
		if (xQueueReceive(queue_action_cmd, &ctrl, portMAX_DELAY) != pdTRUE) {
			vTaskDelay(50);
			continue;
		}

		switch (ctrl.cmd)
		{
		case ACTION_DIZZY:
			layer_dizz->reset();
			layer_dizz->enable();
			eye_send_cmd(EYE_MODE_DIZZY);
			motion_send_cmd(MOTION_CMD_DIZZY, 16);

			if (app_sr_take_mic_sem(1000) != ESP_OK) {
				ESP_LOGE(TAG, "Failed to pause sr mic task");
				continue;
			}
			M5.Mic.end();
			M5.Speaker.begin();
			M5.Speaker.setVolume(settings_get()->volume);
			wav_player.playFromSD("/dizzy.wav");
			M5.Speaker.end();
			M5.Mic.begin();
			
			eye_send_cmd(EYE_MODE_NORMAL);
			layer_dizz->disable();

			vTaskDelay(1000);
			app_sr_give_mic_sem();
			break;
		
		default:
			break;
		}

		
		
		vTaskDelay(50);
	}
}

void app_action_task_init()
{
	queue_action_cmd = xQueueCreate(8, sizeof(app_common_cmd_t));

	static StaticTask_t task_tcb;
    static uint8_t * task_stack = (uint8_t*)heap_caps_malloc(1024 * 4, MALLOC_CAP_SPIRAM);
	xTaskCreateStatic(app_action_task, "ActionTask", 1024 * 4, NULL, 10,  task_stack, &task_tcb);

	// xTaskCreatePinnedToCore(&app_action_task, "ActionTask", 4 * 1024, NULL, 10, NULL, 1);
}

esp_err_t action_send_cmd(uint8_t cmd, int data)
{
	if (!queue_action_cmd) {
		return ESP_FAIL;
	}

	app_common_cmd_t motion;
	motion.cmd = cmd;
	motion.data = data;
	if (xQueueSend(queue_action_cmd, &motion, 0) != pdTRUE) {
		return ESP_FAIL;
	}
	return ESP_OK;
}