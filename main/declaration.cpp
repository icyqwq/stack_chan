#include "declaration.h"

// GoogleTTS tts("AIzaSyAON20OiLXb6qNQvNMR9L9dsqH-DgXmU8U");
// Whisper stt("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");
// ChatGPT gpt("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");
WavPlayer wav_player;
Dynamixel2Arduino dxl(Serial2, -1);
servo servo_1(dxl, 1);
servo servo_2(dxl, 2);
camera cam;
LEDStrip led(RMT_CHANNEL_0, (gpio_num_t)7, 12);
LGFX_Sprite base_canvas(&M5.Display);

TaskHandle_t _play_task_handle;
static bool _play_done = false;
static void _play_task(void *args)
{
	char *dst = (char *)args;
	ESP_LOGI("_play_task", "Start playing %s", dst);
	wav_player.playFromSD(dst);
	free(dst);
	_play_done = true;
	_play_task_handle = NULL;
	vTaskDelete(NULL);
}

bool wav_player_done(void)
{
	return _play_done;
}

void wav_player_play_threaded(String path)
{
	if (_play_task_handle) {
		ESP_LOGE("_play_task", "Previous task not yet done");
		return;
	}
	char * _p = (char*)malloc(path.length() + 1);
	memcpy(_p, path.c_str(), path.length());
	_p[path.length()] = '\0';
	_play_done = false;
	xTaskCreatePinnedToCore(&_play_task, "_play_task", 3 * 1024, _p, 8, &_play_task_handle, 1);
}