#pragma once

#include "anime_png.h"


class AnimeMouth : public AnimePNG
{
public:


protected:
    uint8_t _direction;
    int _mouth_y;
    bool _is_talking;
    
public:
    AnimeMouth(const uint8_t *buffer, int size, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info = "AnimeMouth");
    AnimeMouth(const char *file_name, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info = "AnimeMouth");
    ~AnimeMouth();
    void update(uint64_t frame_counter, LGFX_Sprite &sprite);
    void reset();

    void startTalk()
    {
        _is_talking = true;
    }

    void endTalk()
    {
        _is_talking = false;
    }

protected:

};

inline AnimeMouth::AnimeMouth(const char *file_name, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info):
AnimePNG(file_name, x, y, w, h, loops, id, info)
{
    reset();
}

inline AnimeMouth::AnimeMouth(const uint8_t *buffer, int size, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info): 
AnimePNG(buffer, size, x, y, w, h, loops, id, info)
{
    reset();
}

inline AnimeMouth::~AnimeMouth()
{
}

inline void AnimeMouth::reset()
{
    AnimePNG::reset();
    _is_talking = false;
    _direction = 1;
    _mouth_y = 0;
}

inline void AnimeMouth::update(uint64_t frame_counter, LGFX_Sprite &sprite)
{
    if (_p_png == nullptr) {
        ESP_LOGE(TAG, "Anime not init");
        return;
    }

    uint8_t *p_img = _p_buffer;
    if (!_p_buffer) {
        fpng_decode_memory(_p_png, _png_size, _w, _h);
        convert_rgb888_to_rgb565(fpng_get_buffer(), _w * _h * 3, false);
        p_img = fpng_get_buffer();
    }
    
    if (!_is_talking) {
        sprite.setSwapBytes(true);
        if (_direction) { // move down
            _mouth_y += 2;
            sprite.pushImage(_x, _y + _mouth_y, _w, _h, (uint16_t*)p_img);
            if (_mouth_y >= 6) {
                _direction = 0;
            }
        }
        else {
            _mouth_y -= 2;
            sprite.pushImage(_x, _y + _mouth_y, _w, _h, (uint16_t*)p_img);
            if (_mouth_y <= 0) {
                _direction = 1;
            }
        }
    }
    else {
        sprite.setSwapBytes(false);
        float scale_x = random(80, 120) / 100.0f;
        float scale_y = random(80, 120) / 100.0f;

        int act_w = _w * scale_x;
        int offset_x = (_w - act_w) / 2;

        int act_h = _h * scale_y;
        int offset_y = (_h - act_h) / 2;

        sprite.pushImageRotateZoomWithAA(_x + offset_x, _y + _mouth_y + offset_y, 0, 0, 0, scale_x, scale_y, _w, _h, (uint16_t*)p_img);
    }

    return;
}
