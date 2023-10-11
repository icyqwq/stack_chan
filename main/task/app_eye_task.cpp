#include "app_task.h"
#include "app_common.h"
#include "anime.h"



static QueueHandle_t queue_eye_cmd = NULL;
static float degree1 = 0;
static float degree2 = 360;
static int dx, dy;
static uint32_t next_wait_time = 1;
static uint32_t action_count = 0;
static uint8_t current_mode = EYE_MODE_NORMAL;

void app_eye_normal()
{
	int sel = esp_random() % 20;

	switch (sel)
	{
	case 0:
		layer_eye_left->setDstPos(15, 0);
		layer_eye_right->setDstPos(-15, 0);
		break;

	case 1:
		layer_eye_left->setDstPos(-15, 0);
		layer_eye_right->setDstPos(15, 0);
		break;
	
	default:
		int dx = random(-15, 15);
		int dy = random(-15, 15);
		layer_eye_left->setDstPos(dx, dy);
		layer_eye_right->setDstPos(dx, dy);
	}
	next_wait_time = random(3000, 10000);
}

void app_eye_dizzy()
{
	convertToXY(0, 0, 15, degree1, &dx, &dy);
	layer_eye_left->setDstPos(dx, dy);
	convertToXY(0, 0, 15, degree2, &dx, &dy);
	layer_eye_right->setDstPos(dx, dy);

	degree1 += 20;
	if (degree1 >= 360) {
		degree1 = 0;
	}
	degree2 -= 20;
	if (degree2 <= 0) {
		degree2 = 360;
	}
	next_wait_time = 100;

	
	if (action_count > 50) {
		current_mode = EYE_MODE_NORMAL;
	}
}

void app_eye_task(void *args)
{
	app_common_cmd_t ctrl;
	while (1)
	{
		if (xQueueReceive(queue_eye_cmd, &ctrl, next_wait_time) != pdTRUE) {
			action_count++;
		} else {
			action_count = 0;
			current_mode = ctrl.cmd;
		}
		
		switch (current_mode)
		{
		case EYE_MODE_NORMAL: {
			app_eye_normal();
			break;
		}
		case EYE_MODE_DIZZY: {
			app_eye_dizzy();
			break;
		}
		case EYE_MODE_HIDE: {
			layer_eye_left->disable();
			layer_eye_right->disable();
			next_wait_time = portMAX_DELAY;
			break;
		}
		case EYE_MODE_PAUSE: {
			next_wait_time = portMAX_DELAY;
			break;
		}
		}
	}
}

void app_eye_task_init()
{
	queue_eye_cmd = xQueueCreate(8, sizeof(app_common_cmd_t));

	static StaticTask_t task_tcb;
    static uint8_t * task_stack = (uint8_t*)heap_caps_malloc(1024 * 4, MALLOC_CAP_SPIRAM);
	xTaskCreateStatic(app_eye_task, "LedTask", 1024 * 4, NULL, 1,  task_stack, &task_tcb);
	
	// xTaskCreatePinnedToCore(&app_eye_task, "EyeTask", 3 * 1024, NULL, 1, NULL, 1);
}


void app_eye_stop()
{
	eye_send_cmd(EYE_MODE_PAUSE, 0);
	vTaskDelay(10);
	layer_eye_left->setDstPos(4, 0);
	layer_eye_right->setDstPos(-4, 0);
}

void app_eye_start()
{
	layer_eye_left->enable();
    layer_eye_right->enable();
	eye_send_cmd(EYE_MODE_NORMAL, 0);
}


esp_err_t eye_send_cmd(uint8_t cmd, int data)
{
	if (!queue_eye_cmd) {
		return ESP_FAIL;
	}

	app_common_cmd_t motion;
	motion.cmd = cmd;
	motion.data = data;
	if (xQueueSend(queue_eye_cmd, &motion, 0) != pdTRUE) {
		return ESP_FAIL;
	}
	return ESP_OK;
}