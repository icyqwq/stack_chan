#include "esp_idf_version.h"
#include "string.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_partition.h"

#if ESP_IDF_VERSION_MAJOR == 5
#include "spi_flash_mmap.h"
#else
#include "esp_spi_flash.h"
#endif

#define TAG "EZFILES"
#define EZFILE_PART_NAME "ezfiles"
#define HEAD_MAX_SIZE 4096

static const void *g_ezfiles_ptr = NULL;

typedef spi_flash_mmap_handle_t ezfile_handle_t;

// CRC-16 / MODBUS -> 8005H
const uint16_t crctalbeabs[] = { 
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 
	0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400 
};
 
static uint16_t crc16tablefast(const uint8_t *data, uint32_t len)
{
	uint16_t crc = 0xffff; 
	const uint8_t *ptr = data;
	uint32_t i;
	uint8_t ch;
 
	for (i = 0; i < len; i++) {
		ch = *ptr++;
		crc = crctalbeabs[(ch ^ crc) & 15] ^ (crc >> 4);
		crc = crctalbeabs[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
	} 
	
	return crc;
}

void qmsd_ezfiles_init() {
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, EZFILE_PART_NAME);
    assert(partition != NULL);
    assert(partition->size <= (4 * 1024 * 1024));
    spi_flash_mmap_handle_t map_handle;
    esp_err_t err = esp_partition_mmap(partition, 0, partition->size, SPI_FLASH_MMAP_DATA, &g_ezfiles_ptr, &map_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "partition mmap failed, error code: 0x%x", err);
    }
}

// | mark_valid |   name   | data_offset | data_size |   CRC   |
// |   1 byte   |  35 byte |    8 byte   |   4 byte  |  2byte  |
void qmsd_ezfiles_get(const char* file_name, const uint8_t** ptr, uint32_t* ptr_size) {
    if (g_ezfiles_ptr == NULL) {
        if (ptr) {
            *ptr = NULL;
        }
        if (ptr_size) {
            *ptr_size = 0;
        }
        return ;
    }

    const char* find_ptr = g_ezfiles_ptr;
    for (;;) {
        // got mark useful
        if (find_ptr[0] == 0x01) {
            if (strcmp(file_name, find_ptr + 1) == 0) {
                if (ptr) {
                    *ptr = (uint8_t *)(g_ezfiles_ptr + *(uint32_t*)(find_ptr + 36));
                }
                if (ptr_size) {
                    *ptr_size = *(uint32_t *)(find_ptr + 44);
                }
                return ;
            }
        }
        // Found exits data 
        if (find_ptr[0] == 0xff && find_ptr[1] == 0xff) {
            break ;
        }
        find_ptr += 50;

        // In end exits
        if ((find_ptr - (const char *)g_ezfiles_ptr) >= HEAD_MAX_SIZE) {
            break ;
        }
    }
}

uint8_t qmsd_ezfiles_file_valid(const char* file_name) {
    if (g_ezfiles_ptr == NULL) {
        return 0;
    }

    const uint8_t* ptr = NULL;
    uint32_t ptr_size = 0;
    uint16_t crc_point = 0;
    const char* find_ptr = g_ezfiles_ptr;
    for (;;) {
        // got mark useful
        if (find_ptr[0] == 0x01) {
            if (strcmp(file_name, find_ptr + 1) == 0) {
                ptr = (uint8_t *)(g_ezfiles_ptr + *(uint32_t*)(find_ptr + 36));
                ptr_size = *(uint32_t *)(find_ptr + 44);
                crc_point = *(uint16_t *)(find_ptr + 48);
                break; ;
            }
        }
        // Found exits data 
        if (find_ptr[0] == 0xff && find_ptr[1] == 0xff) {
            break ;
        }
        find_ptr += 50;

        // In end exits
        if ((find_ptr - (const char *)g_ezfiles_ptr) >= HEAD_MAX_SIZE) {
            break ;
        }
    }

    if (ptr == NULL && ptr_size == 0) {
        return 0;
    }

    uint16_t crc_calc = 0;
    crc_calc = crc16tablefast(ptr, ptr_size);
    return crc_calc == crc_point;
}