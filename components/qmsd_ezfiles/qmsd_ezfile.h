#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void qmsd_ezfiles_init();

void qmsd_ezfiles_get(const char* file_name, const uint8_t** ptr, uint32_t* ptr_size);

uint8_t qmsd_ezfiles_file_valid(const char* file_name);

#ifdef __cplusplus
}
#endif
