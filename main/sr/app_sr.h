/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <stdbool.h>
#include <sys/queue.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_models.h"


typedef struct {
    wakenet_state_t wakenet_mode;
    esp_mn_state_t state;
    int command_id;
} sr_result_t;


esp_err_t app_sr_start(bool record_en);
esp_err_t app_sr_stop(void);
esp_err_t app_sr_get_result(sr_result_t *result, TickType_t xTicksToWait);
esp_err_t app_sr_take_mic_sem(uint32_t block_time);
esp_err_t app_sr_give_mic_sem();

float get_threshold_rms();

