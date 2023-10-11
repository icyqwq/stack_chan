#pragma once

#include "anime_png.h"
#include "app_common.h"
#include "anime_moveable_obj.h"

class AnimeMoveablePNG : public AnimePNG, public AnimeMoveableObj
{
public:
    AnimeMoveablePNG(const uint8_t *buffer, int size, int x, int y, int w, int h, int dst_x, int dst_y, float speed, int replay_gap, int32_t id, std::string info = "AnimeMoveablePNG"): AnimePNG(buffer, size, x, y, w, h, ANIME_LOOP_INFINITY, id, info), AnimeMoveableObj(x, y, dst_x, dst_y, speed, replay_gap)
     {
        _dst_x = dst_x;
        _dst_y = dst_y;
        _dx = dst_x - x;
        _dy = dst_y - y;
        _speed = speed;
        _replay_gap = replay_gap;
        reset();
    }
    AnimeMoveablePNG(const char *file_name, int x, int y, int w, int h, int dst_x, int dst_y, float speed, int replay_gap, int32_t id, std::string info = "AnimeMoveablePNG"):
    AnimePNG(file_name, x, y, w, h, ANIME_LOOP_INFINITY, id, info), AnimeMoveableObj(x, y, dst_x, dst_y, speed, replay_gap) {
        _dst_x = dst_x;
        _dst_y = dst_y;
        _dx = dst_x - x;
        _dy = dst_y - y;
        _speed = speed;
        _replay_gap = replay_gap;
        reset();
    }

    ~AnimeMoveablePNG() {};

    void update(uint64_t frame_counter, LGFX_Sprite &sprite)
    {
        if (!_started) {
            return;
        }
        AnimeMoveableObj::update(frame_counter);
        
        if (_p_buffer) {
            sprite.pushImage(_cur_x, _cur_y, _w, _h, (uint16_t*)_p_buffer);
        } else {
            fpng_decode_memory(_p_png, _png_size, _w, _h);
            convert_rgb888_to_rgb565(fpng_get_buffer(), _w * _h * 3, false);
            sprite.setSwapBytes(true);
            sprite.pushImage(_cur_x, _cur_y, _w, _h, (uint16_t*)fpng_get_buffer(), (uint16_t)0x1FF8);
        }
    }

    void reset()
    {
        AnimeBase::reset();
        AnimeMoveableObj::reset();
    }

protected:

};
