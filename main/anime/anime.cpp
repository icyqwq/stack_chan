#include "anime.h"
#include <math.h>
#include "resources.h"
#include "qmsd_ezfile.h"
#define TAG "anime"

anime_play_cmd_t a_ctrl;

uint8_t *anime_png_buffer_face;
uint8_t *anime_png_buffer_mouth;
uint8_t *anime_png_buffer_sweat;
uint8_t *anime_png_buffer_sweat_mirror;
uint8_t *anime_png_buffer_hand;
uint8_t *anime_png_buffer_hand_mirror;
uint8_t *anime_png_buffer_heart;
uint8_t *anime_png_buffer_kawai;

AnimeLog *layer_log;

// layer 0
AnimePNG *layer_face_base;
AnimeMouth *layer_mouth;
AnimeEye *layer_eye_left;
AnimeEye *layer_eye_right;
AnimeMoveablePNG *layer_sweat_0;
AnimeMoveablePNG *layer_sweat_1;
AnimeMoveablePNG *layer_sweat_2;
AnimeMoveablePNG *layer_sweat_3;
AnimeMoveablePNG *layer_hand_left;
AnimeMoveablePNG *layer_hand_right;
AnimePNG *layer_exclamation_mark;

// layer 1
AnimePNG *layer_face_happy;
AnimeRotateZoomPNG *layer_heart_0;
AnimeRotateZoomPNG *layer_heart_1;
AnimeRotateZoomPNG *layer_heart_2;

AnimeSeq *layer_face_cry;

AnimeRotateZoomPNG *layer_radiation;
AnimePNG *layer_face_kawai_left;
AnimePNG *layer_face_kawai_right;

void anime_init()
{    
    fpng_buffer_init(320, 240);

    layer_log = new AnimeLog(2, 2, ANIME_LOOP_INFINITY, ANIME_LOG, "log");

    layer_face_base = new AnimePNG("face-base.png", 0, 0, 320, 240, ANIME_LOOP_INFINITY, ANIME_FACE_BASE, "face-base");
    layer_mouth = new AnimeMouth("mouth.png", 115, 150, 90, 32, ANIME_LOOP_INFINITY, ANIME_MOUTH_BASE, "mouth");
    layer_eye_left = new AnimeEye(80, 112, ANIME_LOOP_INFINITY, ANIME_EYE_LEFT, "eyeL");
    layer_eye_right = new AnimeEye(240, 112, ANIME_LOOP_INFINITY, ANIME_EYE_RIGHT, "eyeR");
    layer_sweat_0 = new AnimeMoveablePNG("sweat.png", 10, 30, 23, 30, 10, 100, 0.05, 10, ANIME_SWEAT_0, "sweat0");
    layer_sweat_1 = new AnimeMoveablePNG("sweat.png", 50, 0, 23, 30, 50, 40, 0.05, 10, ANIME_SWEAT_1, "sweat1");
    layer_sweat_2 = new AnimeMoveablePNG("sweat-mirror.png", 245, 0, 23, 30, 245, 20, 0.05, 10, ANIME_SWEAT_2, "sweat2");
    layer_sweat_3 = new AnimeMoveablePNG("sweat-mirror.png", 290, 20, 23, 30, 290, 80, 0.05, 10, ANIME_SWEAT_3, "sweat3");
    layer_hand_left = new AnimeMoveablePNG("hand.png", -90, 240, 90, 90, -5, 155, 0.1, -1, ANIME_HAND_LEFT, "handL");
    layer_hand_right = new AnimeMoveablePNG("hand-mirror.png", 320, 240, 90, 90, 235, 155, 0.1, -1, ANIME_HAND_RIGHT, "handR");
    layer_exclamation_mark = new AnimePNG("exmark.png", 280, 15, 14, 52, ANIME_LOOP_INFINITY, ANIME_EXCLAMATION_MARK, "exclamation_mark");
    layer_face_kawai_left = new AnimePNG("face-kawai.png", 2, 147, 68, 24, ANIME_LOOP_INFINITY, ANIME_FACE_KAWAI_LEFT, "face-kawai-left");
    layer_face_kawai_right = new AnimePNG("face-kawai.png", 250, 147, 68, 24, ANIME_LOOP_INFINITY, ANIME_FACE_KAWAI_RIGHT, "face-kawai-right");

    layer_face_happy = new AnimePNG("face-happy.png", 0, 0, 320, 240, ANIME_LOOP_INFINITY, ANIME_FACE_HAPPY, "face-happy");


    AnimeRotateZoomPNG::anime_rotate_zoom_png_cfg_t cfg_heart_0 = {
        .x = 24, .y = 52, .w = 50, .h = 47, .dst_x = 0, .dst_y = 0, .scale_x = 0.5, .scale_y = 0.5, .angle = 5, 
        .dst_scale_x = 1.5, .dst_scale_y = 1.5, .dst_angle = 160, .rotate_cx = 0, .rotate_cy = 0, .speed = 0.05, .replay_gap = 10,
    };
    layer_heart_0 = new AnimeRotateZoomPNG("heart.png", cfg_heart_0, ANIME_HEART_0, "heart0");
    layer_heart_0->setEaseCallback(ease_out_cubic);

    AnimeRotateZoomPNG::anime_rotate_zoom_png_cfg_t cfg_heart_1 = {
        .x = 180, .y = 44, .w = 50, .h = 47, .dst_x = 320, .dst_y = 0, .scale_x = 0.3, .scale_y = 0.3, .angle = 30, 
        .dst_scale_x = 1.2, .dst_scale_y = 1.2, .dst_angle = 220, .rotate_cx = 0, .rotate_cy = 0, .speed = 0.07, .replay_gap = 6,
    };
    layer_heart_1 = new AnimeRotateZoomPNG("heart.png", cfg_heart_1, ANIME_HEART_1, "heart1");
    layer_heart_1->setEaseCallback(ease_out_cubic);

    AnimeRotateZoomPNG::anime_rotate_zoom_png_cfg_t cfg_heart_2 = {
        .x = 265, .y = 75, .w = 50, .h = 47, .dst_x = 340, .dst_y = 30, .scale_x = 0.4, .scale_y = 0.4, .angle = 60, 
        .dst_scale_x = 0.8, .dst_scale_y = 0.8, .dst_angle = 350, .rotate_cx = 0, .rotate_cy = 0, .speed = 0.08, .replay_gap = 3,
    };
    layer_heart_2 = new AnimeRotateZoomPNG("heart.png", cfg_heart_2, ANIME_HEART_2, "heart2");
    layer_heart_2->setEaseCallback(ease_out_cubic);

    AnimeRotateZoomPNG::anime_rotate_zoom_png_cfg_t cfg_radiation = {
        .x = 160, .y = 110, .w = 220, .h = 220, .dst_x = 160, .dst_y = 110, .scale_x = 1, .scale_y = 1, .angle = 0, 
        .dst_scale_x = 1, .dst_scale_y = 1, .dst_angle = 350, .rotate_cx = 110, .rotate_cy = 110, .speed = 0.05, .replay_gap = 0,
    };
    layer_radiation = new AnimeRotateZoomPNG("radiation.png", cfg_radiation, ANIME_RADIATION, "radiation");

    layer_face_cry = new AnimeSeq("face_cry.bin", 320, 240, ANIME_LOOP_INFINITY, ANIME_FACE_CRY, "face_cry");

    anime_png_buffer_face = (uint8_t*)heap_caps_malloc(320 * 240 * 2, MALLOC_CAP_SPIRAM);
    anime_png_buffer_mouth = (uint8_t*)heap_caps_malloc(90 * 32 * 2, MALLOC_CAP_SPIRAM);
    anime_png_buffer_sweat = (uint8_t*)heap_caps_malloc(23 * 30 * 2, MALLOC_CAP_SPIRAM);
    anime_png_buffer_sweat_mirror = (uint8_t*)heap_caps_malloc(23 * 30 * 2, MALLOC_CAP_SPIRAM);
    anime_png_buffer_hand = (uint8_t*)heap_caps_malloc(90 * 90 * 2, MALLOC_CAP_SPIRAM);
    anime_png_buffer_hand_mirror = (uint8_t*)heap_caps_malloc(90 * 90 * 2, MALLOC_CAP_SPIRAM);
    anime_png_buffer_heart = (uint8_t*)heap_caps_malloc(50 * 47 * 2, MALLOC_CAP_SPIRAM);
    anime_png_buffer_kawai = (uint8_t*)heap_caps_malloc(68 * 24 * 2, MALLOC_CAP_SPIRAM);

    layer_face_base->setFps(10);
    layer_face_base->setPNGBuffer(anime_png_buffer_face);
    layer_mouth->setPNGBuffer(anime_png_buffer_mouth);
    layer_sweat_0->setPNGBuffer(anime_png_buffer_sweat);
    layer_sweat_1->setPNGBuffer(anime_png_buffer_sweat);
    layer_sweat_2->setPNGBuffer(anime_png_buffer_sweat_mirror);
    layer_sweat_3->setPNGBuffer(anime_png_buffer_sweat_mirror);
    layer_hand_left->setPNGBuffer(anime_png_buffer_hand);
    layer_hand_right->setPNGBuffer(anime_png_buffer_hand_mirror);
    layer_face_kawai_left->setPNGBuffer(anime_png_buffer_kawai);
    layer_face_kawai_right->setPNGBuffer(anime_png_buffer_kawai);
    layer_face_kawai_left->disable();
    layer_face_kawai_right->disable();
    layer_exclamation_mark->disable();

    layer_face_happy->setFps(10);
    layer_face_happy->setPNGBuffer(anime_png_buffer_face);
    layer_heart_0->setPNGBuffer(anime_png_buffer_heart);
    layer_heart_1->setPNGBuffer(anime_png_buffer_heart);
    layer_heart_2->setPNGBuffer(anime_png_buffer_heart);

    layer_radiation->setFps(5);
    layer_radiation->setPNGBuffer(anime_png_buffer_face);

    layer_face_cry->setFps(6);


    addToLayer(ANIME_LAYER_NORMAL, layer_face_base);
    addToLayer(ANIME_LAYER_NORMAL, layer_mouth);
    addToLayer(ANIME_LAYER_NORMAL, layer_eye_left);
    addToLayer(ANIME_LAYER_NORMAL, layer_eye_right);
    addToLayer(ANIME_LAYER_NORMAL, layer_sweat_0);
    addToLayer(ANIME_LAYER_NORMAL, layer_sweat_1);
    addToLayer(ANIME_LAYER_NORMAL, layer_sweat_2);
    addToLayer(ANIME_LAYER_NORMAL, layer_sweat_3);
    addToLayer(ANIME_LAYER_NORMAL, layer_exclamation_mark);
    addToLayer(ANIME_LAYER_NORMAL, layer_hand_left);
    // addToLayer(ANIME_LAYER_NORMAL, layer_hand_right);
    addToLayer(ANIME_LAYER_NORMAL, layer_face_kawai_left);
    addToLayer(ANIME_LAYER_NORMAL, layer_face_kawai_right);
    addToLayer(ANIME_LAYER_NORMAL, layer_log);
    setLayerLoop(ANIME_LAYER_NORMAL, ANIME_LOOP_INFINITY);

    addToLayer(ANIME_LAYER_HAPPY, layer_face_happy);
    addToLayer(ANIME_LAYER_HAPPY, layer_mouth);
    addToLayer(ANIME_LAYER_HAPPY, layer_heart_0);
    addToLayer(ANIME_LAYER_HAPPY, layer_heart_1);
    addToLayer(ANIME_LAYER_HAPPY, layer_heart_2);
    addToLayer(ANIME_LAYER_HAPPY, layer_hand_left);
    addToLayer(ANIME_LAYER_HAPPY, layer_hand_right);
    addToLayer(ANIME_LAYER_HAPPY, layer_face_kawai_left);
    addToLayer(ANIME_LAYER_HAPPY, layer_face_kawai_right);
    addToLayer(ANIME_LAYER_HAPPY, layer_log);
    setLayerLoop(ANIME_LAYER_HAPPY, ANIME_LOOP_INFINITY);

    
    addToLayer(ANIME_LAYER_CRY, layer_face_cry);
    addToLayer(ANIME_LAYER_CRY, layer_log);
    setLayerLoop(ANIME_LAYER_CRY, ANIME_LOOP_INFINITY);

    addToLayer(ANIME_LAYER_RADIATION, layer_radiation);
    setLayerLoop(ANIME_LAYER_RADIATION, ANIME_LOOP_INFINITY);

    // layer_heart_0->startMove();
    // layer_heart_1->startMove();
    
    Anime_SystemInit(1);
    Anime_SendCmd(ANIME_PLAY_CMD_RESUME);
    ESP_LOGI(TAG, "Anime resource init done, free IRAM: %d", esp_get_free_internal_heap_size());
    ESP_LOGI(TAG, "Anime resource init done, free PRAM: %d", esp_get_free_heap_size());
}

void switch_anime(anime_layer_sel_t anime)
{
    ESP_LOGI(TAG, "new anime layer ID = %d", anime);
    if (anime >= ANIME_LAYER_MAX || anime < 0) {
        return;
    }
    a_ctrl.data_i32 = anime;
    a_ctrl.cmd = ANIME_PLAY_CMD_SWITCH;
    Anime_SendCmd(&a_ctrl);
}
