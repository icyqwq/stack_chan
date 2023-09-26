#pragma once


#define EMBED_FILES_DECLARE(file_name) \
extern uint8_t _binary_##file_name##_start; \
extern uint8_t _binary_##file_name##_end; \
static const uint8_t* _##file_name##_ptr = &_binary_##file_name##_start; \
static uint32_t _##file_name##_size = (uint32_t)&_binary_##file_name##_end - (uint32_t)&_binary_##file_name##_start

