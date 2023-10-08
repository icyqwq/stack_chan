#include "app_task.h"
#include "app_common.h"

static float led_threshold_rms = 0;
static SemaphoreHandle_t led_semaphore = NULL;
static float led_current_rms = 0;
static float led_hue = 0;
static uint8_t led_mode = LED_MODE_RMS;
static uint8_t rotate_led_n = 0;

void led_rotate()
{
	uint8_t set_num = 0;
	led.fill(0, 0, 0);
	while (1)
	{
		uint8_t idx = rotate_led_n + set_num;
		if (idx >= led.num()) {
			idx = idx - led.num();
		}

		led.setHSVColor(idx, led_hue, 99, 40);
		led_hue += 2;
		if (led_hue >= 360) {
			led_hue = 0;
		}
		set_num++;
		if (set_num >= 3) {
			break;
		}
	}
	
	rotate_led_n++;
	if (rotate_led_n >= led.num()) {
		rotate_led_n = 0;
	}
	led.update();
	vTaskDelay(50);
}

void led_rms()
{
	if (led_threshold_rms > 4000) {
		led_threshold_rms -= 20;
	} 

	led_threshold_rms = update_ema_rms(led_threshold_rms, led_current_rms, 0.06);
	led_hue = log_map(led_threshold_rms, 4000, 40000, 0, 360);
	led.fillHSV(led_hue, 99, 40);
	led.update();
	vTaskDelay(50);
}

void app_led_task(void *args)
{
	while (1)
	{
		if (led_mode == LED_MODE_RMS) {
			led_rms();
		}
		else if (led_mode == LED_MODE_ROTATE) {
			led_rotate();
		}
		else {
			led.fill(0, 0, 0);
			vTaskDelay(100);
		}
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

void app_led_set_mode(uint8_t mode)
{
	led_mode = mode;
}