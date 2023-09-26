#pragma once

#include <M5Unified.h>
#include "declaration.h"

void app_led_task_init();
void app_led_task_feed(float rms);

void app_face_tracker_init();

void app_touch_task_init();

void app_eye_stop();
void app_eye_start();
void app_eye_task_init();
