#pragma once

#include "anime_base.h"
#include "app_common.h"

class AnimeLabel : public AnimeBase
{
protected:
    int _x, _y;
    String _value;
    uint32_t _color;
    const lgfx::v1::IFont *_font;

public:
    AnimeLabel(int x, int y, int32_t loops, int32_t id, std::string info = "AnimeLabel"):
    AnimeBase(id, loops)
    {
        _x = x;
        _y = y;
        _font = &fonts::Font2;
        _color = lgfx::color888(0, 0, 0);
        reset();
    }
    ~AnimeLabel() {};

    void clean()
    {
        _value = "";
    }

    void setValue(String log)
    {
        _value = log;
    }

    void setColor(uint32_t color)
    {
        _color = color;
    }

    void setFont(const lgfx::v1::IFont *font)
    {
        _font = font;
    }

    void update(uint64_t frame_counter, LGFX_Sprite &sprite)
    {
        if (_value.length() == 0) {
            return;
        }

        sprite.setTextColor(_color);
        sprite.setFont(_font);
        sprite.setTextSize(1);
        sprite.drawString(_value, _x, _y);
    }

    void reset()
    {
        AnimeBase::reset();
    }

protected:

};
