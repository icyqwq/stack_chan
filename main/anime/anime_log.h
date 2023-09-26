#pragma once

#include "anime_base.h"
#include "app_common.h"

class AnimeLog : public AnimeBase
{
public:


protected:
    int _x, _y;
    String _log;

public:
    AnimeLog(int x, int y, int32_t loops, int32_t id, std::string info = "AnimeLog"):
    AnimeBase(id, loops)
    {
        _x = x;
        _y = y;
        reset();
    }
    ~AnimeLog() {};

    void clean()
    {
        _log = "";
    }

    void setLog(String log)
    {
        _log = log;
    }

    void update(uint64_t frame_counter, LGFX_Sprite &sprite)
    {
        if (_log.length() == 0) {
            return;
        }

        sprite.setTextColor(BLACK, lgfx::color888(0xCD, 0xCC, 0xC6));
        sprite.setFont(&fonts::Font2);
        sprite.drawString(_log, _x, _y);
    }

    void reset()
    {
        AnimeBase::reset();
    }

protected:

};
