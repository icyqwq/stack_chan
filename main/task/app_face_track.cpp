#include <Arduino.h>
#include <M5Unified.h>
#include "camera.hpp"
#include "pid.hpp"
#include "declaration.h"
#include "anime.h"
#include "who_human_face_detection.hpp"

#define TAG "Tracker"

#define Y_MAX 1100
#define Y_MIN 760
#define Y_CENTER (Y_MIN + (Y_MAX - Y_MIN) / 2)
#define Y_DEFAULT	780
#define X_DEFAULT	980


PID pid1(0.4, 0, 0);
PID pid2(0.4, 0, 0);

void face_track_task(void *args)
{
	int dx = 0, dy = 0;
    static QueueHandle_t xQueueAIFrame = NULL;
    static QueueHandle_t xQueueLCDFrame = NULL;
	static QueueHandle_t xQueueResult = NULL;
    camera_fb_t *frame = NULL;
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
	xQueueResult = xQueueCreate(5, sizeof(face_track_result_t));

    register_human_face_detection(xQueueAIFrame, NULL, xQueueResult, NULL, false);
    uint32_t frame_count = 0;
	face_track_result_t result;
	uint32_t lost_count = 999;

    while (1)
    {
        // uint32_t start = millis();
        if (cam.getFb() == ESP_OK)
        {
            xQueueSend(xQueueAIFrame, &cam.fb, portMAX_DELAY);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to capture");
            continue;
        }
        // printf("---\n\n%d\n\n", millis() - start);

        // if (xQueueReceive(xQueueLCDFrame, &frame, 1000))
        // {
        //     // ESP_LOGI(TAG, "frame_count = %d", frame_count++);
        //     M5.Display.startWrite();
        //     M5.Display.setAddrWindow(0, 0, 240, 240);
        //     M5.Display.writePixels((uint16_t *)frame->buf,
        //                         int(frame->len / 2));
        //     M5.Display.endWrite();
        // }

		if (xQueueReceive(xQueueResult, &result, 1000)) {
            if (result.cx != -1) {
                lost_count = 0;
                dx = 120 - result.cx;
                dy = 120 - result.cy;
                servo_1.feed(-dx);
                servo_2.feed(-dy);
                float eye_dx = -dx * 0.13;
                float eye_dy = -dy * 0.13;

                layer_eye_left->setDstPos((int)eye_dx, (int)eye_dy);
                layer_eye_right->setDstPos((int)eye_dx, (int)eye_dy);
                // servo_1.setPosition(servo_1.getPosition() - dx);
                // servo_2.setPosition(servo_2.getPosition() - dy);
                printf("dx: %d, dy: %d, x: %d, y: %d\n", dx, dy, servo_1.getPosition(), servo_2.getPosition());
            }
		    else {
                lost_count++;
                if (lost_count < 25) {
                    int dx2 = -SIGN(dx) * 40;
                    int dy2 = -SIGN(dy) * 20;
                    servo_1.feed(dx2);
                    servo_2.feed(dy2);
                    layer_eye_left->setDstPos(dx2, dy2);
                    layer_eye_right->setDstPos(dx2, dy2);
                    // servo_1.setPosition(servo_1.getPosition() - SIGN(dx) * 40);
                    // servo_2.setPosition(servo_2.getPosition() - SIGN(dy) * 20);
                } else {
                    dx = 0;
                    dy = 0;
                    layer_eye_left->setDstPos(0, 0);
                    layer_eye_right->setDstPos(0, 0);
                    servo_1.backToDefault();
                    servo_2.backToDefault();
                }
			}
		}

        cam.releaseFb();
		delay(10);
    }
}

void app_face_tracker_init()
{
	xTaskCreatePinnedToCore(&face_track_task, "faceTracker", 8 * 1024, NULL, 10, NULL, 1);
	pid1.reset_I();
	pid2.reset_I();
}
