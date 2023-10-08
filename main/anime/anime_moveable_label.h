#pragma once

#include "anime_base.h"
#include "app_common.h"

class AnimeMoveableLabel : public AnimeLabel, public AnimeMoveableObj
{
public:


protected:


public:
    AnimeMoveableLabel(int x, int y, int dst_x, int dst_y, float speed, int replay_gap, int32_t loops, int32_t id, std::string info = "AnimeMoveableLabel"):
    AnimeLabel(x, y, loops, id, info), AnimeMoveableObj(x, y, dst_x, dst_y, speed, replay_gap)
    {
        reset();
    }
    ~AnimeMoveableLabel() {};

    void update(uint64_t frame_counter, LGFX_Sprite &sprite) override
    {
        if (_value.length() == 0) {
            return;
        }
        if (!_started) {
            return;
        }
        AnimeMoveableObj::update(frame_counter);

        sprite.setTextColor(_color);
        sprite.setFont(_font);
        sprite.setTextSize(1);
        sprite.drawString(_value, _cur_x, _cur_y);
    }

    void reset() override
    {
        AnimeLabel::reset();
        AnimeMoveableObj::reset();
    }

protected:

};
