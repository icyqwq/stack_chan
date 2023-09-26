#pragma once

#include <M5Unified.h>
#include <string>
#include "qmsd_ezfile.h"
#include "fpng.h"

#define ANIME_LOOP_INFINITY         -1
#ifndef SAFE_FREE
    #define SAFE_FREE(x)	{if (x)	free(x);	(x) = 0;}	/* helper macro */
#endif

class AnimeBase
{
public:
    enum
    {
        ANIME_MODE_JPEG = 0,
        ANIME_MODE_RGB565 = 1,
        ANIME_MODE_PNG = 2,
    };

    // runtime para
    int32_t remain_loops;

protected:
    static constexpr char* TAG = "Anime";
    int32_t _loops;
    uint8_t _fps;
    uint8_t _mode;

    // runtime para
    int64_t _next_update_frame;
    bool _is_running;
    bool _is_enabled;

public:
    int32_t id;
    std::string info;
    virtual void update(uint64_t frame_counter, LGFX_Sprite &sprite) = 0;
    virtual void reset();
    virtual void start();

    void enable() {_is_enabled = true;}
    void disable() {_is_enabled = false;}
    bool isEnabled() {return _is_enabled;}

    uint8_t getFps() {return _fps;}
    void setFps(uint8_t fps) {_fps = fps;}
    bool isRunning() {return _is_running;}

    void base_update(uint64_t frame_counter, LGFX_Sprite &sprite)
    {
        if (!_is_enabled) {
            return;
        }
        update(frame_counter, sprite);
    }

public:
    AnimeBase(int32_t id, int32_t loops);
    virtual ~AnimeBase();
};

inline AnimeBase::AnimeBase(int32_t id, int32_t loops)
{
    _is_enabled = true;
    this->id = id;
    _fps = 30;
    _mode = ANIME_MODE_JPEG;
    _loops = loops;
    reset();
}

inline AnimeBase::~AnimeBase()
{
}

inline void AnimeBase::start()
{
    _is_running = 1;
}

inline void AnimeBase::reset()
{
    _next_update_frame = 0;
    remain_loops = _loops;
    _is_running = false;
}
