#include "who_human_face_detection.hpp"

#include "esp_log.h"
#include "esp_camera.h"

#include "dl_image.hpp"
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"

#include "who_ai_utils.hpp"

#define TWO_STAGE_ON 0

static const char *TAG = "human_face_detection";

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueEvent = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueResult = NULL;

static bool gEvent = true;
static bool gReturnFB = true;

static void task_process_handler(void *arg)
{
    camera_fb_t *frame = NULL;
    HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
#if TWO_STAGE_ON
    HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);
#endif

    while (true)
    {
        if (gEvent)
        {
            bool is_detected = false;
            if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY))
            {
#if TWO_STAGE_ON
                std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
                std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);
#else
                std::list<dl::detect::result_t> &detect_results = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
#endif
                
                if (detect_results.size() > 0)
                {
                    int maxArea = 0;
                    dl::detect::result_t *maxResult = nullptr;

                    for (auto &result : detect_results)
                    {
                        int w = result.box[2] - result.box[0];
                        int h = result.box[3] - result.box[1];
                        int area = w * h;
                        if (area > maxArea)
                        {
                            maxArea = area;
                            maxResult = &result;
                        }
                    }


                    if (xQueueResult)
                    {
                        face_track_result_t max_result;
                        max_result.w = (maxResult->box[2] - maxResult->box[0]);
                        max_result.h = (maxResult->box[3] - maxResult->box[1]);
                        max_result.cx = maxResult->box[0] + (max_result.w >> 1);
                        max_result.cy = maxResult->box[1] + (max_result.h >> 1);
                        
                        // ESP_LOGI("maxResult", "(%3d, %3d, %3d, %3d)", max_result.cx, max_result.cy, max_result.w, max_result.h);

                        xQueueSend(xQueueResult, &max_result, 10);
                    }

                    if (xQueueFrameO)
                    {
                        draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
                    }
                    // print_detection_result(detect_results);
                    is_detected = true;
                }
                else
                {
                    face_track_result_t none_result;
                    none_result.cx = -1;
                    xQueueSend(xQueueResult, &none_result, 10);
                }
            }

            if (xQueueFrameO)
            {
                xQueueSend(xQueueFrameO, &frame, 1000);
            }
            else if (gReturnFB)
            {
                esp_camera_fb_return(frame);
            }
            else
            {
                // free(frame);
            }
        }
    }
}

static void task_event_handler(void *arg)
{
    while (true)
    {
        xQueueReceive(xQueueEvent, &(gEvent), portMAX_DELAY);
    }
}

void register_human_face_detection(const QueueHandle_t frame_i,
                                   const QueueHandle_t event,
                                   const QueueHandle_t result,
                                   const QueueHandle_t frame_o,
                                   const bool camera_fb_return)
{
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    xQueueEvent = event;
    xQueueResult = result;
    gReturnFB = camera_fb_return;

    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 6, NULL, 1);
    if (xQueueEvent)
        xTaskCreatePinnedToCore(task_event_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);
}
