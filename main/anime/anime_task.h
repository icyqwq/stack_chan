#ifndef ANIME_TASK_H
#define ANIME_TASK_H

#include <M5Unified.h>
#include "anime_base.h"
#include <vector>
#include <iostream>
#include <functional>

#define ANIME_FRAME_RATE            60
#define ANIME_FRAME_PERIOD_MS       (1000 / ANIME_FRAME_RATE)
#define ANIME_FRAME_PERIOD_US       (1000000 / ANIME_FRAME_RATE)
#define ANIME_FRAME_PERIOD_TICKS    (ANIME_FRAME_PERIOD_MS / portTICK_PERIOD_MS)
#define ANIME_LOOP_INFINITY         -1

typedef enum
{
    ANIME_PLAY_CMD_SLEEP = 0,
    ANIME_PLAY_CMD_PAUSE,
    ANIME_PLAY_CMD_RESUME,
    ANIME_PLAY_CMD_SWITCH,
} anime_play_cmd_list_t;

typedef enum
{
    ANIME_LAYER_NORMAL = 0,
    ANIME_LAYER_HAPPY,
    ANIME_LAYER_CRY,
    ANIME_LAYER_RADIATION,
    ANIME_LAYER_MAX,
} anime_layer_sel_t;

typedef struct
{
    union {
        void *arg;
        int data_i32;
    };
    anime_play_cmd_list_t cmd;
} anime_play_cmd_t;

typedef struct 
{
    std::vector<AnimeBase*> layers;
    int32_t remain_loops;
} anime_layer_t;

extern anime_layer_t anime_layers[ANIME_LAYER_MAX];

void Anime_SystemInit(uint8_t core_id);
esp_err_t Anime_SendCmd(anime_play_cmd_t *ctrl);
esp_err_t Anime_SendCmd(anime_play_cmd_list_t cmd);
int getAnimeLayerCount();
esp_err_t suspendAnimeTask(uint32_t timeout);
void resumeAnimeTask();
void addToLayer(anime_layer_sel_t idx, AnimeBase* anime);
void setLayerLoop(anime_layer_sel_t idx, int loop);
void setSwapBytes(bool swap);
anime_layer_sel_t getCurrentLayer();

#endif // ANIME_TASK_H