/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "app_sr.h"

#include "esp_mn_speech_commands.h"
#include "esp_process_sdkconfig.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_models.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_iface.h"
#include "esp_mn_iface.h"
#include "app_sr_handler.h"
#include "model_path.h"
#include "app_common.h"
#include "app_task.h"

#include "settings.h"
#include "declaration.h"
#include <M5Unified.h>
#include <SD.h>

static const char *TAG = "app_sr";
int16_t *audio_buffer = NULL;
int audio_buffer_size = 0;

typedef struct {
    model_iface_data_t *model_data;
    const esp_afe_sr_iface_t *afe_handle;
    esp_afe_sr_data_t *afe_data;
    int16_t *afe_in_buffer;
    int16_t *afe_out_buffer;
    TaskHandle_t feed_task;
    TaskHandle_t detect_task;
    TaskHandle_t handle_task;
    QueueHandle_t result_que;
    EventGroupHandle_t event_group;
    SemaphoreHandle_t mic_semaphore;
    StaticTask_t handle_task_tcb;
    uint8_t * handle_task_stack = NULL;

    FILE *fp;
} sr_data_t;

static esp_afe_sr_iface_t *afe_handle = NULL;
static srmodel_list_t *models         = NULL;

static sr_data_t *g_sr_data = NULL;

#define I2S_CHANNEL_NUM (2)
#define NEED_DELETE     BIT0
#define FEED_DELETED    BIT1
#define DETECT_DELETED  BIT2

static float threshold_rms = 2000;

float get_threshold_rms()
{
    return threshold_rms;
}

static void audio_feed_task(void *arg) {
    size_t bytes_read           = 0;
    esp_afe_sr_data_t *afe_data = (esp_afe_sr_data_t *)arg;
    int audio_chunksize         = afe_handle->get_feed_chunksize(afe_data);
    int feed_channel            = 3;
    ESP_LOGI(TAG, "audio_chunksize=%d, feed_channel=%d", audio_chunksize,
             feed_channel);

    /* Allocate audio buffer and check for result */
    audio_buffer =
        (int16_t *)heap_caps_malloc(audio_chunksize * sizeof(int16_t) * feed_channel,
                         MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    audio_buffer_size = audio_chunksize * sizeof(int16_t) * feed_channel;
    if (NULL == audio_buffer) {
        esp_system_abort("No mem for audio buffer");
    }
    g_sr_data->afe_in_buffer = audio_buffer;

    
    // char wav_header_fmt[WAVE_HEADER_SIZE];
    // uint32_t flash_rec_time = BYTE_RATE * 5;
    // uint32_t flash_wr_size = 0;
    // generate_wav_header(wav_header_fmt, flash_rec_time, 16000);

    // auto f_record = SD.open("/record.wav", "w");
    // f_record.write((uint8_t*)wav_header_fmt, WAVE_HEADER_SIZE);
    // f_record.close();

    for (int dummy = 0; dummy < 4; dummy++) {
        // skip noise at start
        M5.Mic.record(audio_buffer, audio_chunksize * I2S_CHANNEL_NUM, 16000, true);
    }
    threshold_rms = calculate_rms(audio_buffer, audio_chunksize * I2S_CHANNEL_NUM, I2S_CHANNEL_NUM);
    ESP_LOGI(TAG, "audio_chunksize * I2S_CHANNEL_NUM = %d", audio_chunksize * I2S_CHANNEL_NUM);
    
    while (true) {
        if (NEED_DELETE && xEventGroupGetBits(g_sr_data->event_group)) {
            xEventGroupSetBits(g_sr_data->event_group, FEED_DELETED);
            vTaskDelete(NULL);
        }
        vTaskDelay(1);
        app_sr_take_mic_sem(portMAX_DELAY);

        /* Read audio data from I2S bus */
        M5.Mic.record(audio_buffer, audio_chunksize * I2S_CHANNEL_NUM, 16000, true);

        app_sr_give_mic_sem();

        float rms = calculate_rms(audio_buffer, audio_chunksize * I2S_CHANNEL_NUM, I2S_CHANNEL_NUM);
        app_led_task_feed(rms);
        if (rms > 300000) {
            action_send_cmd(ACTION_DIZZY);
        }

        threshold_rms = update_ema_rms(threshold_rms, rms, 0.01);
                
        // printf("rms: %.1f, ema: %.1f\n", rms, threshold_rms);

        /* Channel Adjust */
        for (int i = audio_chunksize - 1; i >= 0; i--) {
            audio_buffer[i * 3 + 2] = 0;
            audio_buffer[i * 3 + 1] = audio_buffer[i * 2 + 1];
            audio_buffer[i * 3 + 0] = audio_buffer[i * 2 + 0];
        }

        /* Feed samples of an audio stream to the AFE_SR */
        afe_handle->feed(afe_data, audio_buffer);
    }
}

static void audio_detect_task(void *arg) {
    bool detect_flag            = false;
    esp_afe_sr_data_t *afe_data = (esp_afe_sr_data_t *)arg;

    ESP_LOGI(TAG, "------------detect start------------\n");

    while (true) {
        if (NEED_DELETE && xEventGroupGetBits(g_sr_data->event_group)) {
            xEventGroupSetBits(g_sr_data->event_group, DETECT_DELETED);
            vTaskDelete(g_sr_data->handle_task);
            vTaskDelete(NULL);
        }

        afe_fetch_result_t *res = afe_handle->fetch(afe_data);
        if (!res || res->ret_value == ESP_FAIL) {
            continue;
        }

        if (res->wakeup_state == WAKENET_DETECTED) {
            ESP_LOGI(TAG, LOG_BOLD(LOG_COLOR_GREEN) "wakeword detected");
            sr_result_t result = {
                .wakenet_mode = WAKENET_DETECTED,
                .state        = ESP_MN_STATE_DETECTING,
                .command_id   = 0,
            };
            xQueueSend(g_sr_data->result_que, &result, 0);
        } 
    }
    /* Task never returns */
    vTaskDelete(NULL);
}

esp_err_t app_sr_start(bool record_en) {
    esp_err_t ret = ESP_OK;
    BaseType_t ret_val;
    afe_config_t afe_config = AFE_CONFIG_DEFAULT();
    esp_afe_sr_data_t *afe_data = NULL;
    sys_param_t *param = NULL;
    char *wn_name = NULL;

    ESP_RETURN_ON_FALSE(NULL == g_sr_data, ESP_ERR_INVALID_STATE, TAG,
                        "SR already running");

    g_sr_data = (sr_data_t *)heap_caps_calloc(1, sizeof(sr_data_t),
                                 MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_NO_MEM, TAG,
                        "Failed create sr data");

    g_sr_data->mic_semaphore = xSemaphoreCreateBinary();
    app_sr_give_mic_sem();

    g_sr_data->result_que = xQueueCreate(3, sizeof(sr_result_t));
    ESP_GOTO_ON_FALSE(NULL != g_sr_data->result_que, ESP_ERR_NO_MEM, err, TAG,
                      "Failed create result queue");

    g_sr_data->event_group = xEventGroupCreate();
    ESP_GOTO_ON_FALSE(NULL != g_sr_data->event_group, ESP_ERR_NO_MEM, err, TAG,
                      "Failed create event_group");

    models                  = esp_srmodel_init("model");
    afe_handle              = (esp_afe_sr_iface_t *)&ESP_AFE_SR_HANDLE;
    afe_config = AFE_CONFIG_DEFAULT();

    afe_config.wakenet_model_name =
        esp_srmodel_filter(models, ESP_WN_PREFIX, NULL);
    afe_config.aec_init = false;

    afe_data = afe_handle->create_from_config(&afe_config);
    g_sr_data->afe_handle       = afe_handle;
    g_sr_data->afe_data         = afe_data;

    param = settings_get();

    wn_name = esp_srmodel_filter(models, ESP_WN_PREFIX, "wn9_customword");
    g_sr_data->afe_handle->set_wakenet(g_sr_data->afe_data, wn_name);
    ESP_LOGI(TAG, "load wakenet:%s", wn_name);

    ret_val =
        xTaskCreatePinnedToCore(&audio_feed_task, "Feed Task", 4 * 1024,
                                (void *)afe_data, 12, &g_sr_data->feed_task, 0);
    ESP_GOTO_ON_FALSE(pdPASS == ret_val, ESP_FAIL, err, TAG,
                      "Failed create audio feed task");

    // static StaticTask_t task_tcb_sr_detect;
    // static uint8_t * task_stack_sr_detect = (uint8_t*)heap_caps_malloc(1024 * 8, MALLOC_CAP_SPIRAM);
	// g_sr_data->detect_task = xTaskCreateStatic(audio_detect_task, "Detect", 1024 * 8, (void *)afe_data, 12,  task_stack_sr_detect, &task_tcb_sr_detect);

    ret_val = xTaskCreatePinnedToCore(&audio_detect_task, "Detect Task",
                                      8 * 1024, (void *)afe_data, 12,
                                      &g_sr_data->detect_task, 1);
    ESP_GOTO_ON_FALSE(pdPASS == ret_val, ESP_FAIL, err, TAG,
                      "Failed create audio detect task");

    g_sr_data->handle_task_stack  = (uint8_t*)heap_caps_malloc(1024 * 64, MALLOC_CAP_SPIRAM);
    g_sr_data->handle_task = xTaskCreateStatic(sr_handler_task, "SR Handler", 1024 * 64, NULL, 14,  g_sr_data->handle_task_stack, &g_sr_data->handle_task_tcb);

    // ret_val = xTaskCreatePinnedToCore(&sr_handler_task, "SR Handler Task",
    //                                   32 * 1024, NULL, 15,
    //                                   &g_sr_data->handle_task, 0);
    // ESP_GOTO_ON_FALSE(pdPASS == ret_val, ESP_FAIL, err, TAG,
    //                   "Failed create audio handler task");

    return ESP_OK;
err:
    app_sr_stop();
    return ret;
}

esp_err_t app_sr_stop(void) {
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG,
                        "SR is not running");

    /**
     * Waiting for all task stoped
     * TODO: A task creation failure cannot be handled correctly now
     * */
    xEventGroupSetBits(g_sr_data->event_group, NEED_DELETE);
    xEventGroupWaitBits(g_sr_data->event_group,
                        NEED_DELETE | FEED_DELETED | DETECT_DELETED, 1, 1,
                        portMAX_DELAY);

    if (g_sr_data->result_que) {
        vQueueDelete(g_sr_data->result_que);
        g_sr_data->result_que = NULL;
    }

    if (g_sr_data->event_group) {
        vEventGroupDelete(g_sr_data->event_group);
        g_sr_data->event_group = NULL;
    }

    if (g_sr_data->fp) {
        fclose(g_sr_data->fp);
        g_sr_data->fp = NULL;
    }

    if (g_sr_data->afe_data) {
        g_sr_data->afe_handle->destroy(g_sr_data->afe_data);
    }

    if (g_sr_data->afe_in_buffer) {
        heap_caps_free(g_sr_data->afe_in_buffer);
    }

    if (g_sr_data->afe_out_buffer) {
        heap_caps_free(g_sr_data->afe_out_buffer);
    }

    heap_caps_free(g_sr_data);
    g_sr_data = NULL;
    return ESP_OK;
}

esp_err_t app_sr_get_result(sr_result_t *result, TickType_t xTicksToWait) {
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG,
                        "SR is not running");

    xQueueReceive(g_sr_data->result_que, result, xTicksToWait);
    return ESP_OK;
}

esp_err_t app_sr_take_mic_sem(uint32_t block_time)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG,
                        "SR is not running");
    if (xSemaphoreTake(g_sr_data->mic_semaphore, block_time) != pdTRUE) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t app_sr_give_mic_sem()
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG,
                        "SR is not running");
    if (xSemaphoreGive(g_sr_data->mic_semaphore) != pdTRUE) {
        return ESP_FAIL;
    }
    return ESP_OK;
}
