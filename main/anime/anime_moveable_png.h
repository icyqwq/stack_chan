#pragma once

#include "anime_png.h"
#include "app_common.h"

class AnimeMoveablePNG : public AnimePNG
{
public:
    typedef std::function<float(float)> ease_callback_t;

protected:
    int _dst_x, _dst_y;
    int _cur_x, _cur_y;
    int _dx, _dy;
    float _t = 0;
    int _gap = 0;
    bool _started = false;
    float _speed;
    int _replay_gap;
    ease_callback_t _ease_cb = nullptr;

public:
    AnimeMoveablePNG(const uint8_t *buffer, int size, int x, int y, int w, int h, int dst_x, int dst_y, float speed, int replay_gap, int32_t id, std::string info = "AnimeMoveablePNG"):
    AnimePNG(buffer, size, x, y, w, h, ANIME_LOOP_INFINITY, id, info) {
        _dst_x = dst_x;
        _dst_y = dst_y;
        _dx = dst_x - x;
        _dy = dst_y - y;
        _speed = speed;
        _replay_gap = replay_gap;
        reset();
    }
    AnimeMoveablePNG(const char *file_name, int x, int y, int w, int h, int dst_x, int dst_y, float speed, int replay_gap, int32_t id, std::string info = "AnimeMoveablePNG"):
    AnimePNG(file_name, x, y, w, h, ANIME_LOOP_INFINITY, id, info) {
        _dst_x = dst_x;
        _dst_y = dst_y;
        _dx = dst_x - x;
        _dy = dst_y - y;
        _speed = speed;
        _replay_gap = replay_gap;
        reset();
    }

    ~AnimeMoveablePNG() {};

    void setEaseCallback(ease_callback_t callback)
	{
		_ease_cb = callback;
	}

    void startMove()
    {
        reset();
        _started = true;
    }

    void endMove()
    {
        _started = false;
    }

    void update(uint64_t frame_counter, LGFX_Sprite &sprite)
    {
        if (!_started) {
            return;
        }
        if (_t < 1) {
            float ease_value = _t;
            if (_ease_cb) {
                ease_value = _ease_cb(_t);
            }

            _cur_x = _x + ease_value * _dx;
            _cur_y = _y + ease_value * _dy;
            _t += _speed;
        } else {
            if (_replay_gap >= 0) {
                _gap++;
                if (_gap >= _replay_gap) {
                    _gap = 0;
                    _t = 0;
                }
            }
        }
        
        if (_p_buffer) {
            sprite.pushImage(_cur_x, _cur_y, _w, _h, (uint16_t*)_p_buffer);
        } else {
            fpng_decode_memory(_p_png, _png_size, _w, _h);
            convert_rgb888_to_rgb565(fpng_get_buffer(), _w * _h * 3, false);
            sprite.setSwapBytes(true);
            sprite.pushImage(_cur_x, _cur_y, _w, _h, (uint16_t*)fpng_get_buffer());
        }
    }

    void reset()
    {
        AnimeBase::reset();
        _started = false;
        _cur_x = _x;
        _cur_y = _y;
        _t = 0;
    }

protected:

};
