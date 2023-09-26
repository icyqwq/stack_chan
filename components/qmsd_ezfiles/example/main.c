#include <stdio.h>
#include "qmsd_ezfile.h"

#define TAG "QMSD-MAIN"

void app_main(void)
{   
    qmsd_ezfiles_init();

    const uint8_t* data_ptr = NULL;
    uint32_t data_length = 0;
    qmsd_ezfiles_get("ezfile.txt", &data_ptr, &data_length);
    if (data_ptr) {
        printf("Read data: %.*s\r\n", data_length, data_ptr);
    } else {
        printf("File not found\r\n");
    }
}
