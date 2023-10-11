#include "anime_task.h"
#include "anime.h"
#include "declaration.h"

#define TAG "anime_task"

typedef enum
{
    ANIME_STATE_IDEL = 0,
    ANIME_STATE_SLEEP,
    ANIME_STATE_INIT,
    ANIME_STATE_PLAYING,
} anime_play_state_t;

QueueHandle_t anime_ctrl_queue = NULL;
anime_layer_t anime_layers[ANIME_LAYER_MAX];
anime_layer_sel_t current_layer = ANIME_LAYER_NORMAL;
anime_layer_sel_t last_layer = ANIME_LAYER_NORMAL;
uint32_t t_frame_period_ms = 0;
anime_play_state_t anime_play_state = ANIME_STATE_IDEL;
SemaphoreHandle_t operation_semaphore;
bool _anime_swap;

TaskHandle_t anime_task_handle;
TaskHandle_t anime_ctrl_task_handle;

anime_layer_sel_t getCurrentLayer()
{
    return current_layer;
}

void addToLayer(anime_layer_sel_t idx, AnimeBase* anime)
{
    anime_layers[idx].layers.push_back(anime);
}

void setLayerLoop(anime_layer_sel_t idx, int loop)
{
    anime_layers[idx].remain_loops = loop;
}

void resetCurrentLayer()
{
    for (int i = 0; i < anime_layers[current_layer].layers.size(); i++) {
        anime_layers[current_layer].layers.at(i)->reset();
    }
}

void startCurrentLayer()
{
    base_canvas.fillSprite(0);
    for (int i = 0; i < anime_layers[current_layer].layers.size(); i++) {
        anime_layers[current_layer].layers.at(i)->start();
    }
}

void AnimeExec_Switch(int idx)
{
    if (idx < 0 || idx >= ANIME_LAYER_MAX) {
        ESP_LOGW(TAG, "Invalid layer ID = %d", idx);
        return;
    }

    resetCurrentLayer();
    last_layer = current_layer;
    current_layer = (anime_layer_sel_t)idx;
    t_frame_period_ms = (uint32_t)(1000.0f / (float)(anime_layers[current_layer].layers.at(0)->getFps()));
    startCurrentLayer();
    anime_play_state = ANIME_STATE_PLAYING;
}

void Anime_ControlTask(void *args)
{
    anime_play_cmd_t play_cmd = {0};
    while (1)
    {
        if (xQueueReceive(anime_ctrl_queue, &play_cmd, portMAX_DELAY) == pdTRUE)
        {
            // xSemaphoreTake(operation_semaphore, portMAX_DELAY);
            // ESP_LOGW(TAG, "%d, %p\n", play_cmd.cmd, play_cmd.arg); // TODO: why cause stack overflow ???
            switch (play_cmd.cmd)
            {
            case ANIME_PLAY_CMD_SLEEP:
            {
                anime_play_state = ANIME_STATE_SLEEP;
                break;
            }

            case ANIME_PLAY_CMD_PAUSE:
            {
                anime_play_state = ANIME_STATE_IDEL;
                break;
            }

            case ANIME_PLAY_CMD_RESUME:
            {
                anime_play_state = ANIME_STATE_INIT;
                break;
            }

            case ANIME_PLAY_CMD_SWITCH:
            {
                if (play_cmd.data_i32 == current_layer) {
                    ESP_LOGW(TAG, "Switch to same layer will be ignored");
                    break;
                }
                // suspendAnimeTask();
                if (xSemaphoreTake(operation_semaphore, 1000) != pdTRUE) {
                    ESP_LOGE(TAG, "Failed to suspend anime task");
                }
                vTaskDelay(10);
                AnimeExec_Switch(play_cmd.data_i32);
                
                anime_play_state = ANIME_STATE_PLAYING;
                // resumeAnimeTask();
                xSemaphoreGive(operation_semaphore);
                break;
            }

            default:
                break;
            }   

            // xSemaphoreGive(operation_semaphore);
        }
        else
        {
            continue;
        }
    }
}

void setSwapBytes(bool swap)
{
    _anime_swap = swap;
}

void Anime_Task(void *args)
{
    esp_err_t result;
    
    int64_t t_start = 0, t_delta;
    uint64_t frame_counter = 0;

    while (1)
    {
        t_start = millis();
        xSemaphoreTake(operation_semaphore, portMAX_DELAY);
    
        frame_counter += 1;
        switch (anime_play_state)
        {
        case ANIME_STATE_IDEL:
        {
            vTaskDelay(10);
            break;
        }

        case ANIME_STATE_SLEEP:
        {
            base_canvas.fillSprite(0);
            base_canvas.pushSprite(0, 0);
            anime_play_state = ANIME_STATE_IDEL;
            break;
        }

        case ANIME_STATE_INIT:
        {
            if (anime_layers[current_layer].layers.size() != 0)
            {
                startCurrentLayer();
                t_frame_period_ms = (uint32_t)(1000.0f / (float)(anime_layers[current_layer].layers.at(0)->getFps()));
                anime_play_state = ANIME_STATE_PLAYING;
            }
            break;
        }

        case ANIME_STATE_PLAYING:
        {
            if (anime_layers[current_layer].layers.size() == 0)
            {
                anime_play_state = ANIME_STATE_SLEEP;
                break;
            }

            for (int i = 0; i < anime_layers[current_layer].layers.size(); i++) {
                uint32_t start = millis();
                anime_layers[current_layer].layers.at(i)->base_update(frame_counter, base_canvas);
                // ESP_LOGI(TAG, "Layer %d, Use %d ms", i, millis() - start);
            }

            base_canvas.pushSprite(0, 0);
            // M5.Display.startWrite();
            // M5.Display.setAddrWindow(0, 0, 320, 240);
            // M5.Display.writePixels((uint16_t*)base_canvas.getBuffer(), 320*240, _anime_swap);
            // M5.Display.endWrite();

            if (anime_layers[current_layer].remain_loops > 0)
            {
                anime_layers[current_layer].remain_loops--;
            }
            else if (anime_layers[current_layer].remain_loops == 0)
            {
                anime_play_cmd_t a_ctrl;
                a_ctrl.cmd = ANIME_PLAY_CMD_SWITCH;
                a_ctrl.data_i32 = last_layer;
                Anime_SendCmd(&a_ctrl);
                vTaskDelay(10);
            }
            
            break;
        }

        default:
            break;
        }

        xSemaphoreGive(operation_semaphore);

        t_delta = millis() - t_start;

        if (anime_play_state == ANIME_STATE_PLAYING)
        {
            if (t_delta > t_frame_period_ms)
            {
                ESP_LOGW(TAG, "Frame rate unstable, %lld ms used, %lld ms exceed.", t_delta, t_delta - t_frame_period_ms);
                vTaskDelay(1);
            }
            else
            {
                // ESP_LOGI(TAG, "frame %llu, delta: %lld, delay: %lld", frame_counter, t_delta, (ANIME_FRAME_PERIOD_MS - t_delta));
                // uint32_t a = m_millis();
                vTaskDelay(t_frame_period_ms - t_delta);
                // uint32_t b = m_millis() - a;
                // ESP_LOGI(TAG, "%d delayed", b);
            }
            // vTaskDelay(10);
        }
    }
}

esp_err_t suspendAnimeTask(uint32_t timeout)
{
    // vTaskSuspend(anime_task_handle);
    // vTaskSuspend(anime_ctrl_task_handle);
    if (xSemaphoreTake(operation_semaphore, timeout) != pdTRUE) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

void resumeAnimeTask()
{
    // vTaskResume(anime_task_handle);
    // vTaskResume(anime_ctrl_task_handle);
    xSemaphoreGive(operation_semaphore);
}

void Anime_SystemInit(uint8_t core_id)
{
    anime_ctrl_queue = xQueueCreate(5, sizeof(anime_play_cmd_t));

    if (anime_ctrl_queue == NULL)
    {
        ESP_LOGE(TAG, "failed to create anime control queue");
        return;
    }

    operation_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(operation_semaphore);

    // static StaticTask_t task_tcb;
    // static uint8_t * task_stack = (uint8_t*)heap_caps_malloc(1024 * 8, MALLOC_CAP_SPIRAM);
	// xTaskCreateStatic(Anime_Task, "Anime_Task", 1024 * 8, NULL, 2,  task_stack, &task_tcb);

    static StaticTask_t task_tcb2;
    static uint8_t * task_stack2 = (uint8_t*)heap_caps_malloc(1024 * 4, MALLOC_CAP_SPIRAM);
	xTaskCreateStatic(Anime_ControlTask, "Anime_ControlTask", 1024 * 4, NULL, 3,  task_stack2, &task_tcb2);
    
    xTaskCreatePinnedToCore(&Anime_Task, "Anime_Task", 8 * 1024, NULL, 1, &anime_task_handle, core_id);
    // xTaskCreatePinnedToCore(&Anime_ControlTask, "Anime_ControlTask", 4 * 1024, NULL, 2, &anime_ctrl_task_handle, core_id);
}

esp_err_t Anime_SendCmd(anime_play_cmd_t *ctrl)
{
    if (anime_ctrl_queue == NULL) {
        return ESP_FAIL;
    }
    if (xQueueSend(anime_ctrl_queue, ctrl, 0) == pdFALSE)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t Anime_SendCmd(anime_play_cmd_list_t cmd)
{
    if (anime_ctrl_queue == NULL) {
        return ESP_FAIL;
    }
    anime_play_cmd_t ctrl;
    ctrl.cmd = cmd;
    if (xQueueSend(anime_ctrl_queue, &ctrl, 0) == pdFALSE)
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

int getAnimeLayerCount()
{
    return anime_layers[current_layer].layers.size();
}
