#pragma once

#include "app_common.h"

class AnimeMoveableObj
{
public:
    typedef std::function<float(float)> ease_callback_t;

protected:
	int _x, _y;
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
    AnimeMoveableObj(int x, int y, int dst_x, int dst_y, float speed, int replay_gap) {
        _x = x;
        _y = y;
        _dst_x = dst_x;
        _dst_y = dst_y;
        _dx = dst_x - x;
        _dy = dst_y - y;
        _speed = speed;
        _replay_gap = replay_gap;
        reset();
    }

    ~AnimeMoveableObj() {};

    void setEaseCallback(ease_callback_t callback)
	{
		_ease_cb = callback;
	}

	bool moveStarted()
    {
        return _started;
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

    void update(uint64_t frame_counter)
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
    }

    void reset()
    {
        _started = false;
        _cur_x = _x;
        _cur_y = _y;
        _t = 0;
    }

protected:

};
