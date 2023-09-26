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