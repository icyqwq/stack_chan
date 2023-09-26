#define ARDUINOJSON_DECODE_UNICODE 1
#include <Arduino.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include <M5Unified.h>
#include <SD.h>

#include "app_sr.h"
#include "declaration.h"
#include "app_common.h"

#include "app_task.h"
#include "resources.h"
#include "anime.h"

#include "app_motion.h"
#include "api_caiyun.hpp"
#include "api_openweather.hpp"
#define TAG "MAIN"

const char *ssid = "Real-Internet";
const char *pswd = "ENIAC2333";

void connect_wifi()
{
    WiFi.begin(ssid, pswd);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void weather_api_test(void *args)
{
    // CaiyunAPI api;
    OpenWeatherAPI api;

    for (int i = 0; i < 1; i++) {
        api.test();
    }

    while (1)
    {
        delay(1000);
    }
}


extern "C" void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);


    // disableCore0WDT();
    // disableCore1WDT();
    M5.begin();

    connect_wifi();
    xTaskCreatePinnedToCore(&weather_api_test, "weather_api_test", 64 * 1024, NULL, 10, NULL, 0);
    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
    
    
    ESP_LOGI("MAIN", "\n\nHELLO\n\n");
    SD.begin(GPIO_NUM_4, SPI, 25000000);

    M5.Display.setBrightness(100);

    M5.Speaker.begin();
    M5.Speaker.setVolume(50);
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
    M5.Speaker.end();


    qmsd_ezfiles_init();

    ESP_LOGI(TAG, "SD Card: %d, %d MiB", SD.cardType(), SD.cardSize() / 1024 / 1024);

    // BM8563 Init (clear INT)
    M5.In_I2C.writeRegister8(0x51, 0x00, 0x00, 100000L);
    M5.In_I2C.writeRegister8(0x51, 0x01, 0x00, 100000L);
    M5.In_I2C.writeRegister8(0x51, 0x0D, 0x00, 100000L);

    // AW9523 Control BOOST
    M5.In_I2C.bitOn(0x58, 0x03, 0b00000000, 100000L); // BOOST_EN

    dxl.begin(57600, 5, 6);
    dxl.setPortProtocolVersion((float)2.0);
    if (!servo_1.ping()) {
        ESP_LOGE(TAG, "Servo 1 not found");
    }
    if (!servo_2.ping()) {
        ESP_LOGE(TAG, "Servo 2 not found");
    }
    if (servo_1.setup(8192, -8192, 100) != ESP_OK) {
        ESP_LOGE(TAG, "Servo 1 setup failed");
    }
    if (servo_2.setup(1100, 760, 100) != ESP_OK) {
        ESP_LOGE(TAG, "Servo 2 setup failed");
    }
    servo_1.torqueOn();
    servo_2.torqueOn();
    servo_1.setupDefaultPosition(1024);
    servo_2.setupDefaultPosition(800);
    servo_1.backToDefault();
    servo_2.backToDefault();
    // servo_test();
    
    motion_init();

    // base_canvas.setColorDepth(lgfx::rgb565_2Byte);
    base_canvas.setBuffer(heap_caps_malloc(320 * 240 * 3, MALLOC_CAP_SPIRAM), 320, 240, lgfx::rgb565_2Byte);
    // base_canvas.setPsram(true);
    // base_canvas.createSprite(320, 240);

    led.begin();
    // lcd_test();
    cam.init(FRAMESIZE_QVGA);


    anime_init();

    
    // app_face_tracker_init();
    

    settings_read_from_nvs();
    ESP_LOGI(TAG, "settings_get()->volume: %d", settings_get()->volume);

    layer_log->setLog("Connecting to " + String(ssid));
    connect_wifi();

    app_touch_task_init();
    app_led_task_init();
    app_eye_task_init();
    
    layer_log->setLog("Starting SR");
    M5.Speaker.end();
    M5.Mic.begin();
    M5.Mic.end();
    app_sr_start(false);

    layer_log->clean();

    // startSystemStatsTask();

    // switch_anime(ANIME_LAYER_CRY);
    // delay(3000);
    // switch_anime(ANIME_LAYER_RADIATION);
    // layer_radiation->startMove();

    char buf[20];
    while (1)
    {        
        if (getCurrentLayer() == ANIME_LAYER_NORMAL) {
            if (M5.Power.isCharging()) {
                layer_face_kawai_left->enable();
                layer_face_kawai_right->enable();
            } else {
                layer_face_kawai_left->disable();
                layer_face_kawai_right->disable();
            }
            printf("%d, %d\n", M5.Power.isCharging(), M5.Power.getBatteryVoltage());
        }
        
        vTaskDelay(2000);
    }
}
