#include "app_task.h"
#include "app_common.h"
#include "anime.h"

static SemaphoreHandle_t eye_semaphore = NULL;


void app_eye_task(void *args)
{
	while (1)
	{
		xSemaphoreTake(eye_semaphore, portMAX_DELAY);
		
		int sel = esp_random() % 10;

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

		xSemaphoreGive(eye_semaphore);

		vTaskDelay(random(3000, 10000));
	}
}

void app_eye_task_init()
{
	eye_semaphore = xSemaphoreCreateBinary();
	xTaskCreatePinnedToCore(&app_eye_task, "EyeTask", 3 * 1024, NULL, 1, NULL, 1);
	xSemaphoreGive(eye_semaphore);
}

void app_eye_stop()
{
	if (!eye_semaphore) {
		return;
	}

	xSemaphoreTake(eye_semaphore, 100);
	layer_eye_left->setDstPos(4, 0);
	layer_eye_right->setDstPos(-4, 0);
}

void app_eye_start()
{
	if (!eye_semaphore) {
		return;
	}

	layer_eye_left->enable();
    layer_eye_right->enable();

	xSemaphoreGive(eye_semaphore);
}

