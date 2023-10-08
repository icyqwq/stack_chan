/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <M5Unified.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_check.h"
#include "app_sr.h"
#include "file_manager.h"
#include "audio_player.h"
#include "file_iterator.h"

#include "app_sr_handler.h"
#include "declaration.h"
#include "settings.h"

#include "app_task.h"
#include "app_function_call.h"
#include "app_common.h"
#include "app_motion.h"
#include <SD.h>

#include "anime.h"


static const char *TAG = "sr_handler";

void play_wakeup_audio()
{
    vTaskDelay(10);
    M5.Speaker.begin();
    M5.Speaker.setVolume(settings_get()->volume);
    M5.Speaker.tone(1000, 100);
    M5.delay(100);
    M5.Speaker.tone(2000, 100);
    M5.delay(100);
    M5.Speaker.tone(3000, 100);
    do
    {
        delay(1);
        M5.update();
    } while (M5.Speaker.isPlaying());
    M5.Speaker.stop();
    M5.Speaker.end();
}

void reconnect_wifi()
{
    // WiFi.disconnect();
    // delay(100);
    // WiFi.begin("Real-Internet", "ENIAC2333");
}

#define RECORD_FRAME_LEN    1024 // 100ms
#define MAX_SAMPLE_COUNT    16000 * 10 // 10s


static uint8_t * wav_file_buffer = NULL;
static uint32_t wav_size = 0;
static int16_t * p_audio_buf = NULL;
extern int16_t *audio_buffer;
extern int audio_buffer_size;
static Whisper::language_t chat_lang = Whisper::ENGLISH;
ChatGPT gpt("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");

esp_err_t request_stt(String &result)
{
    Whisper stt("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");

    chat_lang = stt.getLastLanguage();
    generate_wav_header((char*)wav_file_buffer, wav_size, 16000, 1);
    wav_size += WAVE_HEADER_SIZE;
    if (stt.request(wav_file_buffer, wav_size) != ESP_OK) {
        wav_player.playFromSD(String("/network_error_") + k_lang_postfix[chat_lang]);
        ESP_LOGE(TAG, "STT API request failed.");
        layer_log->setValue("STT error");
        reconnect_wifi();
        return ESP_FAIL;
    }
    MEMCHECK
    if (stt.last_result.isEmpty()) {
        ESP_LOGW(TAG, "Not talk anything");
        layer_log->setValue("Talk nothing");
        return ESP_FAIL;
    } 

    chat_lang = stt.getLastLanguage();
    printf("getLastLanguage = %d\n", chat_lang);
    result = stt.getLastResult();
    return ESP_OK;
}

esp_err_t request_gpt(String &request, String &result, int &func_id)
{
    if (gpt.request(request) != ESP_OK) {
        wav_player.playFromSD(String("/network_error_") + k_lang_postfix[chat_lang]);
        ESP_LOGE(TAG, "GPT API request failed.");
        layer_log->setValue("GPT error");
        reconnect_wifi();
        return ESP_FAIL;
    }
    func_id = gpt.getFunctionCallID();
    if (func_id != 0) {
        result = gpt.getFunctionCallArg();
    }
    else {
        result = gpt.getLastResult();
    }
    return ESP_OK;
}

esp_err_t request_tts(String &request)
{
    GoogleTTS tts("AIzaSyAON20OiLXb6qNQvNMR9L9dsqH-DgXmU8U");
    
    tts.setSpeakerStartCallback([](int rms) {
        startTalk();
    });

    if (tts.request(request, k_lang_name[chat_lang], k_lang_model[chat_lang]) != ESP_OK) {
        wav_player.playFromSD(String("/network_error_") + k_lang_postfix[chat_lang]);
        ESP_LOGE(TAG, "TTS API request failed.");
        layer_log->setValue("TTS error");
        reconnect_wifi();
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t record_segment()
{
    MEMCHECK
    if (app_sr_take_mic_sem(1000) != ESP_OK) {
        return ESP_FAIL;
    }

    app_eye_stop();
    layer_exclamation_mark->enable();
    layer_hand_left->startMove();
    M5.Mic.end();
    M5.Speaker.begin();
    M5.Speaker.setVolume(settings_get()->volume);
    wav_player.playFromSD("/response.wav");
    M5.Speaker.end();
    // play_wakeup_audio();
    M5.Mic.begin();
    
    // record start
    float rms = 0;
    float ema_rms = get_threshold_rms() * 0.9;
    p_audio_buf = (int16_t*)(wav_file_buffer + WAVE_HEADER_SIZE);
    // M5.Display.fillRect(0, 0, 10, 10, GREEN);

    MEMCHECK
    // STAGE 2: Recording    
    uint32_t inactive_count = 0;
    uint32_t valid_count = 0;
    wav_size = 0;
    for (uint32_t i = 0; i < MAX_SAMPLE_COUNT; i += RECORD_FRAME_LEN) {
        if (M5.Mic.record(audio_buffer, RECORD_FRAME_LEN, 16000, false) == false) {
            ESP_LOGE(TAG, "M5 Mic record error");
        }
        memcpy(p_audio_buf + i, audio_buffer, RECORD_FRAME_LEN * 2);

        wav_size += RECORD_FRAME_LEN;
        rms = calculate_rms(audio_buffer, RECORD_FRAME_LEN, 1); // calculate previous rms, since current recording is not done
        app_led_task_feed(rms);
        ema_rms = update_ema_rms(ema_rms, rms, 0.05);
        printf("[%.1f s] current rms: %.1f, ema: %.1f, threshold: %.1f\n", i / 16000.0, rms, ema_rms, get_threshold_rms());
        if (ema_rms < get_threshold_rms()) { // inactive
            inactive_count++;
            // M5.Display.fillRect(0, 0, 10, 10, YELLOW);
            if (inactive_count > ((24000) / RECORD_FRAME_LEN)) { // 1.5s
                break;
            }
        } else {
            valid_count++;
            inactive_count = 0;
            // M5.Display.fillRect(0, 0, 10, 10, GREEN);
        }

        do
        {
            delay(1);
        } while (M5.Mic.isRecording());
    }
    printf("Record time: %.1f s", wav_size / 16000.0f);
    wav_size *= 2; // convert to bytes
    // M5.Display.fillRect(0, 0, 10, 10, RED);
    layer_exclamation_mark->disable();
    MEMCHECK

    // Echo test
    // M5.Mic.end();
    // M5.Speaker.begin();
    // if (!M5.Speaker.isEnabled())
    // {
    //     M5.Display.print("Speaker not found...");
    //     for (;;) { M5.delay(1); }
    // }
    // M5.Speaker.setVolume(255);
    // M5.Speaker.playRaw(p_audio_buf, wav_size, 16000, false, 1, 0);
    // do
    // {
    //     delay(1);
    //     M5.update();
    // } while (M5.Speaker.isPlaying());
    // M5.Speaker.end();
    // M5.Mic.begin();
    layer_hand_left->endMove();

    if (valid_count == 0) {
        ESP_LOGW(TAG, "Not talk anything");
        app_eye_start();
        app_led_set_mode(LED_MODE_RMS);
        return ESP_FAIL;
    }

    layer_eye_left->disable();
    layer_eye_right->disable();
    layer_sweat_0->startMove();
    layer_sweat_1->startMove();
    layer_sweat_2->startMove();
    layer_sweat_3->startMove();

    M5.Mic.end();
    M5.Speaker.begin();
    M5.Speaker.setVolume(settings_get()->volume);

    String result;
    int func_id;
    MEMCHECK
    if (request_stt(result) != ESP_OK) {
        goto failed;
    }
    MEMCHECK

    if (request_gpt(result, result, func_id) != ESP_OK) {
        goto failed;
    }
    MEMCHECK
    
    if (func_id != 0) {
        if (invoke_function(func_id, result, chat_lang) != ESP_OK) {
            ESP_LOGE(TAG, "Function execution failed.");
            layer_log->setValue("Exec error");
            goto failed;
        }
    }
    else {
        if (result.length() == 0) {
            ESP_LOGE(TAG, "No Response.");
            layer_log->setValue("No response");
            goto failed;
        }
        if (request_tts(result) != ESP_OK) {
            goto failed;
        }
    }

    MEMCHECK

    M5.Speaker.end();
    M5.Mic.begin();
    endTalk();
    app_led_set_mode(LED_MODE_RMS);
    return ESP_OK;
    
    failed:
    switch_anime(ANIME_LAYER_CRY);
    vTaskDelay(3000);
    switch_anime(ANIME_LAYER_NORMAL);
    vTaskDelay(50);
    layer_log->clean();
    M5.Speaker.end();
    M5.Mic.begin();
    endTalk();
    app_led_set_mode(LED_MODE_RMS);

    return ESP_FAIL;
}



void sr_handler_task(void *pvParam) {
    audio_player_state_t last_player_state = AUDIO_PLAYER_STATE_IDLE;

    if (wav_file_buffer == NULL) {
        wav_file_buffer = (uint8_t*)heap_caps_calloc(MAX_SAMPLE_COUNT + WAVE_HEADER_SIZE / 2 + 1 + 16000, sizeof(int16_t), MALLOC_CAP_SPIRAM); // 640K for ~20s recording
    }

    while (true) {
        sr_result_t result;
        app_sr_get_result(&result, portMAX_DELAY);
        char audio_file[48] = {0};

        if (ESP_MN_STATE_TIMEOUT == result.state) {
            printf("Timeout\r\n");
            continue;
        }

        if (WAKENET_DETECTED == result.wakenet_mode) {
            printf("Detected\r\n");
            motion_send_cmd(MOTION_CMD_NOD, 2);
            app_led_set_mode(LED_MODE_ROTATE);
            record_segment();

            app_sr_give_mic_sem();
            continue;
        }
    }
    vTaskDelete(NULL);
}

