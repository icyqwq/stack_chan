#pragma once

#include "driver/rmt.h"
#include "led_strip.h"

class LEDStrip
{
private:
	static constexpr char* TAG = "LED";
	led_strip_t *_strip = NULL;
	uint16_t red = 0;
	uint16_t green = 0;
	uint16_t blue = 0;
	uint8_t color_select = 0;

	rmt_channel_t _channel;
	gpio_num_t _pin;
	uint16_t _num;

public:
	LEDStrip(rmt_channel_t channel, gpio_num_t pin, uint16_t num)
	{
		_channel = channel;
		_pin = pin;
		_num = num;
	}

	~LEDStrip()
	{
	}

	esp_err_t begin(void)
	{
		rmt_config_t config = {
			.rmt_mode = RMT_MODE_TX,
			.channel = _channel,
			.gpio_num = _pin,
			.clk_div = 80,
			.mem_block_num = 1,
			.flags = 0,
			.tx_config = {
				.carrier_freq_hz = 38000,
				.carrier_level = RMT_CARRIER_LEVEL_HIGH,
				.idle_level = RMT_IDLE_LEVEL_LOW,
				.carrier_duty_percent = 33,
				.carrier_en = false,
				.loop_en = false,
				.idle_output_en = true,
			}};
		config.clk_div = 2;
		ESP_ERROR_CHECK(rmt_config(&config));
		ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
		led_strip_config_t strip_config =
			LED_STRIP_DEFAULT_CONFIG(_num, (led_strip_dev_t)config.channel);
		_strip = led_strip_new_rmt_ws2812(&strip_config);
		if (!_strip)
		{
			ESP_LOGE(TAG, "Install led driver failed");
			return ESP_FAIL;
		}
		return ESP_OK;
	}

	void update()
	{
		ESP_ERROR_CHECK(_strip->refresh(_strip, 100));
	}

	void fillHSV(float h, float s, float v)
	{
		if (h < 0) {
			h = 0;
		}
		if (h > 360) {
			h = 360;
		}
		float r = 0, g = 0, b = 0;
        HSVtoRGB(h, s, v, r, g, b);
		fill(r, g, b);
	}
	
	void fill(uint8_t r, uint8_t g, uint8_t b)
	{
		for (size_t i = 0; i < _num; i++)
		{
			ESP_ERROR_CHECK(_strip->set_pixel(_strip, i, r, g, b));
		}
	}

	void setColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
	{
		ESP_ERROR_CHECK(_strip->set_pixel(_strip, n, r, g, b));
	}

	void setHSVColor(uint16_t n, float h, float s, float v)
	{
		if (h < 0) {
			h = 0;
		}
		if (h > 360) {
			h = 360;
		}
		float r = 0, g = 0, b = 0;
        HSVtoRGB(h, s, v, r, g, b);
		ESP_ERROR_CHECK(_strip->set_pixel(_strip, n, r, g, b));
	}

	void HSVtoRGB(float H, float S, float V, float &R, float &G, float &B)
	{
		if (H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0)
		{
			return;
		}
		float s = S / 100.0f;
		float v = V / 100.0f;
		float C = s * v;
		float X = C * (1 - abs(fmod(H / 60.0f, 2) - 1));
		float m = v - C;
		float r, g, b;
		if (H >= 0 && H < 60)
		{
			r = C, g = X, b = 0;
		}
		else if (H >= 60 && H < 120)
		{
			r = X, g = C, b = 0;
		}
		else if (H >= 120 && H < 180)
		{
			r = 0, g = C, b = X;
		}
		else if (H >= 180 && H < 240)
		{
			r = 0, g = X, b = C;
		}
		else if (H >= 240 && H < 300)
		{
			r = X, g = 0, b = C;
		}
		else
		{
			r = C, g = 0, b = X;
		}
		R = (r + m) * 255;
		G = (g + m) * 255;
		B = (b + m) * 255;
	}

	uint16_t num()
	{
		return _num;
	}
};
