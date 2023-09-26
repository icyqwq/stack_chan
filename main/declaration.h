#include <M5Unified.h>
#include "gpt.hpp"
#include "whisper.hpp"
#include "google_tts.hpp"
#include "wav_player.hpp"
#include "servo.hpp"
#include "camera.hpp"
#include "ledstrip.hpp"
#include "who_human_face_detection.hpp"
#include "settings.h"

extern WavPlayer wav_player;
extern Dynamixel2Arduino dxl;
extern servo servo_1;
extern servo servo_2;
extern camera cam;
extern LEDStrip led;
extern LGFX_Sprite base_canvas;