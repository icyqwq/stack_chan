#pragma once

#include "anime_base.h"

#ifndef READ
    #define READ(x, type, p_buffer) x = *((type*)(p_buffer))
#endif

class AnimeSeq : public AnimeBase
{
public:
    typedef struct
    {
        uint8_t im_num;
        uint8_t seq_len;
        uint32_t *p_sizes;
        uint8_t **p_images;
        uint8_t *p_seq;
        uint8_t *p_seq_timing;
    } seq_t;

protected:
    seq_t _seq;

    // runtime para
    int32_t _play_idx;
    uint8_t *_p_next_frame;
    uint32_t _size;
    int _w, _h;

public:
    AnimeSeq(uint8_t *buffer, int w, int h, int32_t loops, int32_t id, std::string info = "AnimeSeq");
    AnimeSeq(const char* file_name, int w, int h, int32_t loops, int32_t id, std::string info = "AnimeSeq"); // load from ezfiles
    ~AnimeSeq();
    virtual void update(uint64_t frame_counter, LGFX_Sprite &sprite);
    virtual void reset();
    uint8_t getSeqLen();

protected:
    uint8_t getNextFrame();
    void init(uint8_t *buffer, int w, int h, int32_t loops, int32_t id, std::string info);
};

inline void AnimeSeq::init(uint8_t *buffer, int w, int h, int32_t loops, int32_t id, std::string info)
{
    ESP_LOGI(TAG, "Init %s", info.c_str());
    _w = w;
    _h = h;
    this->info = info;
    
    _play_idx = 0;

    enum
    {
        offset_magic = 0,
        offset_fps = 4,
        offset_im_num = 5,
        offset_seq_len = 6,
        offset_sizes = 7
    };

    uint8_t *p;

    uint32_t magic;
    READ(magic, uint32_t, buffer + offset_magic);
    if (magic == 0xEB4CE6F1)
    {
        _mode = ANIME_MODE_RGB565;
    }
    else if (magic == 0xEB4CE6F2)
    {
        _mode = ANIME_MODE_JPEG;
    }
    else if (magic == 0xEB4CE6F3)
    {
        _mode = ANIME_MODE_PNG;
    }
    else
    {
        ESP_LOGE(TAG, "Not a valid anime sequence structure.");
        return;
    }

    _fps = buffer[offset_fps];

    if (_fps > 30 && _mode == ANIME_MODE_JPEG)
    {
        ESP_LOGW(TAG, "Framerates greater than 30 may be unstable in JPEG mode");
    }

    _seq.im_num = buffer[offset_im_num];
    _seq.seq_len = buffer[offset_seq_len];
    _seq.p_images = (uint8_t**)calloc(sizeof(uint8_t*), _seq.im_num);
    if ( _seq.p_images == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create anime sequence");
        return;
    }

    ESP_LOGI(TAG, "fps: %d, mode: %s\n", _fps, _mode == ANIME_MODE_RGB565 ? "RGB565" : _mode == ANIME_MODE_JPEG ? "JPEG" : "PNG");
    ESP_LOGI(TAG, "num of images: %d\n", _seq.im_num);
    ESP_LOGI(TAG, "len of sequence: %d\n", _seq.seq_len);
    ESP_LOGI(TAG, "images size: ");

    uint8_t offset_seq = offset_sizes + _seq.im_num * 4;
    uint8_t offset_seq_timing = offset_seq + _seq.seq_len;
    uint8_t offset_images = offset_seq_timing + _seq.seq_len;

    // read sizes
    p = buffer + offset_sizes;
    _seq.p_sizes = (uint32_t*)p;
    for (int i = 0; i < _seq.im_num; i++)
    {
        // READ(_seq.p_sizes[i], uint32_t, p + i * 4);
        printf("%d, ", _seq.p_sizes[i]);
    }

    // read seq
    p = buffer + offset_seq;
    // memcpy((uint8_t*)(_seq.p_seq), (uint8_t*)p, _seq.seq_len);
    _seq.p_seq = p;
    printf("\nsequence: ");
    for (int i = 0; i < _seq.seq_len; i++)
    {
        printf("%d, ", _seq.p_seq[i]);
    }

    // read timing
    p = buffer + offset_seq_timing;
    // _seq.p_seq_timing = (uint8_t*)calloc(sizeof(uint8_t), _seq.seq_len);
    // memcpy(_seq.p_seq_timing, p, _seq.seq_len);
    _seq.p_seq_timing = p;
    printf("\ntiming: ");
    for (int i = 0; i < _seq.seq_len; i++)
    {
        // _seq.p_seq_timing[i] = 10;
        printf("%d, ", _seq.p_seq_timing[i]);
    }
    printf("\n");

    // read images
    p = buffer + offset_images;
    for (int i = 0; i < _seq.im_num; i++)
    {
        _seq.p_images[i] = p;
        p += _seq.p_sizes[i];
    }
}

inline AnimeSeq::AnimeSeq(const char* file_name, int w, int h, int32_t loops, int32_t id, std::string info): AnimeBase(id, loops)
{
    const uint8_t* data_ptr = NULL;
    uint32_t data_length = 0;
    qmsd_ezfiles_get(file_name, &data_ptr, &data_length);
    if (data_ptr) {
        init((uint8_t*)data_ptr, w, h, loops, id, info);
    } else {
        ESP_LOGE(TAG, "EzFile %s not found\r\n", file_name);
    }
}

inline AnimeSeq::AnimeSeq(uint8_t *buffer, int w, int h, int32_t loops, int32_t id, std::string info): AnimeBase(id, loops)
{
    init(buffer, w, h, loops, id, info);
}

inline AnimeSeq::~AnimeSeq()
{
    SAFE_FREE(_seq.p_images);
}

inline void AnimeSeq::update(uint64_t frame_counter, LGFX_Sprite &sprite)
{
    uint8_t time;

    if (_next_update_frame == -1)
    {
        return;
    }

    // not at update time, use last frame
    if (frame_counter < _next_update_frame)
    {
        if (_mode == ANIME_MODE_RGB565)
        {
            memcpy(sprite.getBuffer(), _p_next_frame, _w*_h*2);
        }
        else if (_mode == ANIME_MODE_PNG)
        {
            fpng_decode_memory(_p_next_frame, _size, (uint8_t*)sprite.getBuffer(), _w, _h);
            convert_rgb888_to_rgb565((uint8_t*)sprite.getBuffer(), _w*_h*3, true);
        }
        else
        {
            // canvas->drawJpegImage(canvas, 0, 0, 0, NULL, _p_next_frame, _size, RGB565_LITTLE_ENDIAN, 0);
        }
        return;
    }

    // get next frame to update
    time = getNextFrame();
    // ESP_LOGI(TAG, "time = %d, index = %d", time, _play_idx);

    if (time == 255)
    {
        _next_update_frame = -1; // never access current anime
        return;
    }
    else
    {
        _next_update_frame = frame_counter + time;
    }

    if (_mode == ANIME_MODE_RGB565)
    {
        memcpy(sprite.getBuffer(), _p_next_frame, _w*_h*2);
    }
    else if (_mode == ANIME_MODE_PNG)
    {
        fpng_decode_memory(_p_next_frame, _size, (uint8_t*)sprite.getBuffer(), _w, _h);
        convert_rgb888_to_rgb565((uint8_t*)sprite.getBuffer(), _w*_h*3, true);
    }
    else
    {
        // canvas->drawJpegImage(canvas, 0, 0, 0, NULL, _p_next_frame, _size, RGB565_LITTLE_ENDIAN, 0);
    }
}

inline void AnimeSeq::reset()
{
    AnimeBase::reset();
    _play_idx = 0;
    _p_next_frame = nullptr;
    _size = 0;
    ESP_LOGI(TAG, "Anime seq [%s] reset", this->info.c_str());
}

inline uint8_t AnimeSeq::getNextFrame()
{
    if (_play_idx >= _seq.seq_len) // all frames played
    {
        // ESP_LOGI(TAG, "remain_loops = %d", remain_loops);
        _play_idx = 0;
        // if (_loops == ANIME_LOOP_INFINITY) // keep playing
        // {
        //     _play_idx = 0;
        // }
        // else if (remain_loops > 0)
        // {
        //     _play_idx = 0;
        //     remain_loops--;
        // }
        // else // stop playing
        // {
        //     _p_next_frame = nullptr;
        //     _size = 0;
        //     reset();
        //     return 255; // invalid timing
        // }
        // return _seq.p_seq_timing[_play_idx];
    }
    
    _p_next_frame = _seq.p_images[_seq.p_seq[_play_idx]];
    _size = _seq.p_sizes[_seq.p_seq[_play_idx]];
    uint8_t time = _seq.p_seq_timing[_play_idx];
    _play_idx++;
    return time;
}

inline uint8_t AnimeSeq::getSeqLen()
{
    return _seq.seq_len;
}