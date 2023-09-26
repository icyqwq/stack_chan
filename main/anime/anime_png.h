#pragma once

#include "anime_base.h"

class AnimePNG : public AnimeBase
{
public:


protected:
    const uint8_t *_p_png = nullptr;
    uint32_t _png_size = 0;
    uint8_t *_p_buffer = nullptr;
    int _x, _y, _w, _h;

public:
    AnimePNG(const uint8_t *buffer, int size, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info = "AnimePNG");
    AnimePNG(const char *file_name, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info = "AnimePNG");
    ~AnimePNG();
    void update(uint64_t frame_counter, LGFX_Sprite &sprite);
    void reset();
    void setPNGBuffer(uint8_t *buffer)
    {
        _p_buffer = buffer;
    }

    void start()
    {
        AnimeBase::start();
        if (_p_buffer) {
            fpng_decode_memory(_p_png, _png_size, fpng_get_buffer(), _w, _h);
            convert_rgb888_to_rgb565(fpng_get_buffer(), _w * _h * 3, false);
            memcpy(_p_buffer, fpng_get_buffer(), _w * _h * 2);
        }
    }

protected:

};

inline AnimePNG::AnimePNG(const char *file_name, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info):
AnimeBase(id, loops)
{
    const uint8_t* data_ptr = NULL;
    uint32_t data_length = 0;
    qmsd_ezfiles_get(file_name, &data_ptr, &data_length);
    if (data_ptr) {
        _p_png = data_ptr;
        _png_size = data_length;
        _w = w;
        _h = h;
        _x = x;
        _y = y;
        _p_buffer = nullptr;
        reset();
    } else {
        ESP_LOGE(TAG, "EzFile %s not found\r\n", file_name);
    }
}

inline AnimePNG::AnimePNG(const uint8_t *buffer, int size, int x, int y, int w, int h, int32_t loops, int32_t id, std::string info): 
AnimeBase(id, loops)
{
    _p_buffer = nullptr;
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _p_png = buffer;
    _png_size = size;
    reset();
}

inline AnimePNG::~AnimePNG()
{
}

inline void AnimePNG::reset()
{
    AnimeBase::reset();
}

inline void AnimePNG::update(uint64_t frame_counter, LGFX_Sprite &sprite)
{
    if (_p_png == nullptr) {
        ESP_LOGE(TAG, "Anime not init");
        return;
    }

    if (_p_buffer) {
        sprite.setSwapBytes(true);
        sprite.pushImage(_x, _y, _w, _h, (uint16_t*)_p_buffer);
        return;
    }

    fpng_decode_memory(_p_png, _png_size, _w, _h);
    convert_rgb888_to_rgb565(fpng_get_buffer(), _w * _h * 3, false);
    // M5.Display.setSwapBytes(false);
    sprite.setSwapBytes(true);
    sprite.pushImage(_x, _y, _w, _h, (uint16_t*)fpng_get_buffer());
    
    return;
}
