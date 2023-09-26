#include "app_task.h"
#include "app_common.h"
#include "anime.h"
#include "app_motion.h"

#define TAG "Touch"

void app_touch_task(void *args)
{
	bool _detected = false;
	uint8_t data, ret;
	uint32_t delay_count = 0;
	bool last_kawai_state = false;
	while (1)
	{
		ret = M5.In_I2C.start(0x3E, 1, 50000);
        ret &= M5.In_I2C.read(&data, 1);
        ret &= M5.In_I2C.stop();
		if (ret == false) {
			ESP_LOGW(TAG, "TP read failed");
			vTaskDelay(50);
			continue;
		}
		if (data) {
			printf("Touch data %02X\n", data);
			delay_count = 0;
        	// printf("%02X, ret = %d\n", data, ret);
			if (!_detected) {
				_detected = true;
				switch_anime(ANIME_LAYER_HAPPY);
				vTaskDelay(50); // wait for switch
				layer_heart_0->startMove();
				layer_heart_1->startMove();
				layer_heart_2->startMove();

				last_kawai_state = layer_face_kawai_left->isEnabled();
				layer_face_kawai_left->disable();
				layer_face_kawai_right->disable();
				
				bool hand = false;
				if (random() % 2) {
					layer_hand_left->startMove();
					hand = true;
				}
				if (random() % 2) {
					layer_hand_right->startMove();
					hand = true;
				}
				if (!hand) {
					layer_face_kawai_left->enable();
					layer_face_kawai_right->enable();
				}

				if (app_sr_take_mic_sem(1000) != ESP_OK) {
					ESP_LOGE(TAG, "Failed to pause sr mic task");
					continue;
				}
				M5.Mic.end();
				M5.Speaker.begin();
    			M5.Speaker.setVolume(settings_get()->volume);
				wav_player.playFromSD("/affection.wav");
				M5.Speaker.end();
				M5.Mic.begin();
				app_sr_give_mic_sem();
			}
			motion_send_cmd(MOTION_CMD_NOD, 2);
		} else {
			if (_detected) {
				delay_count++;
				if (delay_count > 20) {
					_detected = false;
					switch_anime(ANIME_LAYER_NORMAL);
					vTaskDelay(50);
					if (last_kawai_state) {
						layer_face_kawai_left->enable();
						layer_face_kawai_right->enable();
					} else {
						layer_face_kawai_left->disable();
						layer_face_kawai_right->disable();
					}
				}
			}
		}
		vTaskDelay(50);
	}
}

void app_touch_task_init()
{
	// led_semaphore = xSemaphoreCreateBinary();
	xTaskCreatePinnedToCore(&app_touch_task, "TouchTask", 4 * 1024, NULL, 3, NULL, 1);
	// led.fillHSV(0, 99, 40);
}