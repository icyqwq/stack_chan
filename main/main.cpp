#define ARDUINOJSON_DECODE_UNICODE 1
#include <Arduino.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include <M5Unified.h>
#include <SD.h>
#include <ESPmDNS.h>


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

void setupWiFi()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.onEvent(_wifi_event);
    WiFi.softAP("M5-StackChan", "whatsoever");

    WiFi.begin(ssid, pswd);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        printf("Connecting to WiFi...\n");
    }
    printf("Connected to WiFi\n");
    printf("STA IP Address: %s\n", WiFi.localIP().toString().c_str());
    printf("AP  IP Address: %s\n", WiFi.softAPIP().toString().c_str());

    if (!MDNS.begin("m5stackchan")) {
        ESP_LOGE(TAG, "Error setting up MDNS responder!\n");
    }
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

void test(void *args)
{
    extern ChatGPT gpt;
    gpt.request("深圳的天气怎么样");
    vTaskDelete(NULL);
}

uint16_t RGB565ToGray(uint16_t rgb565) {
    rgb565 = (rgb565 >> 8) | (rgb565 << 8);
    uint8_t r = (rgb565 >> 11) & 0x1F;  // 取出红色值
    uint8_t g = (rgb565 >> 5) & 0x3F;   // 取出绿色值
    uint8_t b = rgb565 & 0x1F;          // 取出蓝色值

    // 将RGB转为灰度值，这里使用了常见的转换公式
    // 注意：我们需要先将每个通道的值扩展到8位，然后再进行计算
    uint8_t gray = (uint8_t)((r * 255 / 31 * 0.299) + (g * 255 / 63 * 0.587) + (b * 255 / 31 * 0.114));

    // 将8位的灰度值映射回5位和6位的范围
    return ((gray * 31 / 255) << 11) | ((gray * 63 / 255) << 5) | (gray * 31 / 255);
}

#include "rgb565_to_gray_lut.h"
extern "C" void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    M5.begin();

    // setupWiFi();
    // SD.begin(GPIO_NUM_4, SPI, 25000000);
    // vTaskDelay(3000);
    // cam.init(FRAMESIZE_QVGA);
    // app_wifi_task_init();
    // while (1)
    // {
    //         if (cam.getFb() == ESP_OK) {
    //         // M5.Display.startWrite();
    //         // M5.Display.setAddrWindow(0, 0, 320, 240);
    //         // // M5.Display.writePixels((uint16_t*)cam.fb->buf, int(cam.fb->len / 2));
    //         // uint16_t *p = (uint16_t*)cam.fb->buf;
    //         // for (int i = 0; i < int(cam.fb->len / 2); i++) {
    //         //     uint8_t gray = rgb565_to_gray_lut[p[i]];
    //         //     uint16_t c = ((gray * 31 / 255) << 11) | ((gray * 63 / 255) << 5) | (gray * 31 / 255);
    //         //     M5.Display.writeData16(c);
    //         // }
    //         // M5.Display.endWrite();
    //         // vTaskDelay(1000);
    //         ditherImg((uint16_t*)cam.fb->buf, app_wifi_get_img_buffer(), 320, 240);
    //         uint8_t *p = app_wifi_get_img_buffer();

    //         M5.Display.startWrite();
    //         M5.Display.setAddrWindow(0, 0, 320, 240);
            
    //         for (int i = 0; i < 9600; i++) {
    //             for (int j = 0; j < 8; j++) {
    //                 if ((p[i] << j) & 0x80) {
    //                     M5.Display.writeData16(BLACK);
    //                 } else {
    //                     M5.Display.writeData16(WHITE);
    //                 }
    //             }
    //         }
    //         M5.Display.endWrite();
    //     }
    //     else {
    //         M5.Display.fillRect(0, 0, 320, 240, RED);
    //     }
    //     cam.releaseFb();

    //     wifi_send_cmd(WIFI_CMD_SEND_FRAME);

    //     vTaskDelay(10000);
    // }
    
    
    // PAUSE

    ESP_LOGI("MAIN", "\n\nHELLO\n\n");
    SD.begin(GPIO_NUM_4, SPI, 20000000);

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
    settings_get()->volume = 50;

    layer_log->setValue("Connecting to " + String(ssid));
    setupWiFi();

    app_touch_task_init();
    app_led_task_init();
    app_eye_task_init();
    app_action_task_init();
    app_wifi_task_init();
    
    layer_log->setValue("Starting SR");
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
    
    // xTaskCreatePinnedToCore(&test, "test", 16 * 1024, NULL, 10, NULL, 0);

    // vTaskDelay(5000);
    // cam.getFb();
    // cam.releaseFb();
    // if (cam.getFb() == ESP_OK) {
	// 	ditherImg((uint16_t*)cam.fb->buf, app_wifi_get_img_buffer(), 320, 240);
	// 	wifi_send_cmd(WIFI_CMD_SEND_FRAME);
	// }
	// cam.releaseFb();

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
            // printf("%d, %d\n", M5.Power.isCharging(), M5.Power.getBatteryVoltage());
        }
        
        vTaskDelay(2000);
    }
}
