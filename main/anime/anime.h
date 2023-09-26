#pragma once

#include "anime_task.h"
#include "anime_base.h"
#include "anime_seq.h"
#include "anime_png.h"
#include "anime_mouth.h"
#include "anime_eye.h"
#include "anime_moveable_png.h"
#include "anime_rotatezoom_png.h"
#include "anime_log.h"

typedef enum
{
    ANIME_FACE_BASE = 0,
    ANIME_MOUTH_BASE,
    ANIME_EYE_LEFT,
    ANIME_EYE_RIGHT,
    ANIME_SWEAT_0,
    ANIME_SWEAT_1,
    ANIME_SWEAT_2,
    ANIME_SWEAT_3,
    ANIME_HAND_LEFT,
    ANIME_HAND_RIGHT,
    ANIME_EXCLAMATION_MARK,

    ANIME_FACE_HAPPY,
    ANIME_HEART_0,
    ANIME_HEART_1,
    ANIME_HEART_2,

    ANIME_FACE_CRY,

    ANIME_FACE_KAWAI_LEFT,
    ANIME_FACE_KAWAI_RIGHT,

    ANIME_RADIATION,
    ANIME_LOG,
    ANIME_MAX
} anime_sel_t;

extern AnimeLog *layer_log;

extern AnimePNG *layer_face_base;
extern AnimeMouth *layer_mouth;
extern AnimeEye *layer_eye_left;
extern AnimeEye *layer_eye_right;
extern AnimeMoveablePNG *layer_sweat_0;
extern AnimeMoveablePNG *layer_sweat_1;
extern AnimeMoveablePNG *layer_sweat_2;
extern AnimeMoveablePNG *layer_sweat_3;
extern AnimeMoveablePNG *layer_hand_left;
extern AnimeMoveablePNG *layer_hand_right;
extern AnimePNG *layer_exclamation_mark;

extern AnimePNG *layer_face_happy;
extern AnimeRotateZoomPNG *layer_heart_0;
extern AnimeRotateZoomPNG *layer_heart_1;
extern AnimeRotateZoomPNG *layer_heart_2;
extern AnimeRotateZoomPNG *layer_radiation;

extern AnimePNG *layer_face_kawai_left;
extern AnimePNG *layer_face_kawai_right;

void anime_init();
void switch_anime(anime_layer_sel_t anime);

