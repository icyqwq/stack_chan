#pragma once

#include "anime_moveable_png.h"
#include "app_common.h"
#include <functional>

class AnimeRotateZoomPNG : public AnimeMoveablePNG
{
public:
    typedef struct
    {
        int x;
        int y;
        int w;
        int h;
        int dst_x;
        int dst_y;
        float scale_x;
        float scale_y;
        float angle;
        float dst_scale_x;
        float dst_scale_y;
        float dst_angle;
        int rotate_cx;
        int rotate_cy;
        float speed;
        int replay_gap;
    } anime_rotate_zoom_png_cfg_t;
    

protected:
    float _start_scale_x, _start_scale_y, _start_angle;
    float _dst_scale_x, _dst_scale_y, _dst_angle;
    float _cur_scale_x, _cur_scale_y, _cur_angle;
    float _d_scale_x, _d_scale_y, _d_angle;
    float _rotate_cx, _rotate_cy;

public:
    AnimeRotateZoomPNG(const uint8_t *buffer, int size, anime_rotate_zoom_png_cfg_t &cfg, int32_t id, std::string info = "AnimeRotateZoomPNG"):
    AnimeMoveablePNG(buffer, size, cfg.x, cfg.y, cfg.w, cfg.h, cfg.dst_x,cfg.dst_y, cfg.speed, cfg.replay_gap, id, info) {
        _start_scale_x = cfg.scale_x;
        _start_scale_y = cfg.scale_y;
        _start_angle = cfg.angle;
        _dst_scale_x = cfg.dst_scale_x;
        _dst_scale_y = cfg.dst_scale_y;
        _dst_angle = cfg.dst_angle;
        _d_scale_x = cfg.dst_scale_x - cfg.scale_x;
        _d_scale_y = cfg.dst_scale_y - cfg.scale_y;
        _d_angle = cfg.dst_angle - cfg.angle;
        _rotate_cx = cfg.rotate_cx;
        _rotate_cy = cfg.rotate_cy;
        reset();
    }
    AnimeRotateZoomPNG(const char *file_name, anime_rotate_zoom_png_cfg_t &cfg, int32_t id, std::string info = "AnimeRotateZoomPNG"):
    AnimeMoveablePNG(file_name, cfg.x, cfg.y, cfg.w, cfg.h, cfg.dst_x,cfg.dst_y, cfg.speed, cfg.replay_gap, id, info) {
        _start_scale_x = cfg.scale_x;
        _start_scale_y = cfg.scale_y;
        _start_angle = cfg.angle;
        _dst_scale_x = cfg.dst_scale_x;
        _dst_scale_y = cfg.dst_scale_y;
        _dst_angle = cfg.dst_angle;
        _d_scale_x = cfg.dst_scale_x - cfg.scale_x;
        _d_scale_y = cfg.dst_scale_y - cfg.scale_y;
        _d_angle = cfg.dst_angle - cfg.angle;
        _rotate_cx = cfg.rotate_cx;
        _rotate_cy = cfg.rotate_cy;
        reset();
    }

    ~AnimeRotateZoomPNG() {};

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
            _cur_scale_x = _start_scale_x + ease_value * _d_scale_x;
            _cur_scale_y = _start_scale_y + ease_value * _d_scale_y;
            _cur_angle = _start_angle + ease_value * _d_angle;
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

        int act_w = _w * _cur_scale_x;
        int offset_x = (_w - act_w) / 2;

        int act_h = _h * _cur_scale_y;
        int offset_y = (_h - act_h) / 2;
        
        if (_p_buffer) {
            sprite.pushImageRotateZoom(_cur_x + offset_x, _cur_y + offset_y, _rotate_cx, _rotate_cy, _cur_angle, _cur_scale_x, _cur_scale_y, _w, _h, (uint16_t*)_p_buffer);
        } else {
            fpng_decode_memory(_p_png, _png_size, _w, _h);
            convert_rgb888_to_rgb565(fpng_get_buffer(), _w * _h * 3, false);
            sprite.setSwapBytes(true);
            sprite.pushImageRotateZoom(_cur_x + offset_x, _cur_y + offset_y, _rotate_cx, _rotate_cy, _cur_angle, _cur_scale_x, _cur_scale_y, _w, _h, (uint16_t*)fpng_get_buffer());
        }
    }

    void reset()
    {
        AnimeMoveablePNG::reset();
        _cur_scale_x = _start_scale_x;
        _cur_scale_y = _start_scale_y;
        _cur_angle = _start_angle;
    }

protected:

};
