#pragma once

#include "anime_base.h"
#include "app_common.h"

class AnimeEye : public AnimeBase
{
public:


protected:
    int _x, _y;
    int _dst_x, _dst_y;
    int _cur_x, _cur_y;

public:
    AnimeEye(int x, int y, int32_t loops, int32_t id, std::string info = "AnimeEye"):
    AnimeBase(id, loops)
    {
        _x = x;
        _y = y;
        reset();
    }
    ~AnimeEye() {};

    void setDstPos(int dx, int dy)
    {
        if (!is_point_inside_circle(dx, dy, 0, 0, 14)) {
            closest_point_on_circle_edge(dx, dy, 0, 0, 14, &dx, &dy);
        }
        _dst_x = dx;
        _dst_y = dy;
    }

    void update(uint64_t frame_counter, LGFX_Sprite &sprite)
    {
        static const uint8_t k_delta = 4;
        if (_cur_x < _dst_x) {
            _cur_x += k_delta;
            if (_cur_x > _dst_x) {
                _cur_x = _dst_x;
            }
        }
        if (_cur_x > _dst_x) {
            _cur_x -= k_delta;
            if (_cur_x < _dst_x) {
                _cur_x = _dst_x;
            }
        }

        if (_cur_y < _dst_y) {
            _cur_y += k_delta;
            if (_cur_y > _dst_y) {
                _cur_y = _dst_y;
            }
        }
        if (_cur_y > _dst_y) {
            _cur_y -= k_delta;
            if (_cur_y < _dst_y) {
                _cur_y = _dst_y;
            }
        }
        
        sprite.fillCircle(_x + _cur_x, _y + _cur_y, 20, 0);
    }

    void reset()
    {
        AnimeBase::reset();
        _dst_x = 0;
        _dst_y = 0;
        _cur_x = 0;
        _cur_y = 0;
    }

protected:

};
