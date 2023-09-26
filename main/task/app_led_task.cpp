#include "app_task.h"
#include "app_common.h"

static float led_threshold_rms = 0;
static SemaphoreHandle_t led_semaphore = NULL;
static float led_current_rms = 0;
static float led_hue = 0;

void app_led_task(void *args)
{
	while (1)
	{
		if (led_threshold_rms > 4000) {
			led_threshold_rms -= 20;
		} 

		led_threshold_rms = update_ema_rms(led_threshold_rms, led_current_rms, 0.06);
		led_hue = log_map(led_threshold_rms, 4000, 40000, 0, 360);
        led.fillHSV(led_hue, 99, 40);
		vTaskDelay(50);
	}
}

void app_led_task_init()
{
	// led_semaphore = xSemaphoreCreateBinary();
	xTaskCreatePinnedToCore(&app_led_task, "LedTask", 3 * 1024, NULL, 1, NULL, 1);
	// led.fillHSV(0, 99, 40);
}

void app_led_task_feed(float rms)
{
	// if (led_semaphore == NULL) {
	// 	return;
	// }
	led_current_rms = rms;
	// xSemaphoreGive(led_semaphore);
}