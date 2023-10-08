#pragma once

#include "esp_camera.h"
#include <M5Unified.h>


class camera
{
private:
	static constexpr char* TAG = "CAMERA";
public:
	camera_fb_t *fb;

	camera(/* args */) {}
	~camera()
	{
		esp_camera_deinit();
	}

	esp_err_t init(framesize_t frame_size) 
	{
		static camera_config_t camera_config = {
			.pin_pwdn     = -1,
			.pin_reset    = -1,
			.pin_xclk     = 2,
			.pin_sscb_sda = -1, // 12 // set to -1 using existing I2C driver
			.pin_sscb_scl = 11,

			.pin_d7 = 47,
			.pin_d6 = 48,
			.pin_d5 = 16,
			.pin_d4 = 15,
			.pin_d3 = 42,
			.pin_d2 = 41,
			.pin_d1 = 40,
			.pin_d0 = 39,

			.pin_vsync = 46,
			.pin_href  = 38,
			.pin_pclk  = 45,

			.xclk_freq_hz = 20000000,
			.ledc_timer   = LEDC_TIMER_0,
			.ledc_channel = LEDC_CHANNEL_0,

			.pixel_format = PIXFORMAT_RGB565,
			.frame_size   = frame_size,
			.jpeg_quality = 0,
			.fb_count     = 1,
			.fb_location  = CAMERA_FB_IN_PSRAM,
			.grab_mode    = CAMERA_GRAB_WHEN_EMPTY,

			.sccb_i2c_port = I2C_NUM_1
		};

		esp_err_t err = esp_camera_init(&camera_config);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "Camera init failed, err = %d", err);
		}
		return err;
	}

	void deinit()
	{
		esp_camera_deinit();
	}

	esp_err_t getFb()
	{
		fb = esp_camera_fb_get();
		if (!fb) {
			ESP_LOGE(TAG, "Camera capture failed");
			return ESP_FAIL;
		}
		return ESP_OK;
	}

	void releaseFb(void) {
		esp_camera_fb_return(fb);
	}

	void captureToScreen(int x, int y, int w, int h)
	{
		if (getFb() == ESP_OK) {
			M5.Display.startWrite();
			M5.Display.setAddrWindow(x, y, w, h);
			M5.Display.writePixels((uint16_t*)fb->buf,
								int(fb->len / 2));
			M5.Display.endWrite();
		} else {
			M5.Display.fillRect(x, y, w, h, RED);
		}
		releaseFb();
	}

	
};

