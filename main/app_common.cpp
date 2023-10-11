#include "app_common.h"
#include <SD.h>
#include "rgb565_to_gray_lut.h"
#include <map>

bool compareUsage(TaskStatus_t *a, TaskStatus_t *b)
{
    return a->ulRunTimeCounter < b->ulRunTimeCounter;
}

template <class T>
void InsertionSort(bool compare(T *a, T *b), T arr[], int n)
{
	int i, j;
	T temp;

	for (int i = 1; i < n; ++i)
	{

		temp = arr[i];
		j = i - 1;

		while (j >= 0 && compare(&(arr[j]), &temp))
		{
			arr[j + 1] = arr[j];
			j = j - 1;
		}

		arr[j + 1] = temp;

	}
}

void System_StatsTask(void *args)
{
    // stats task
    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxArraySize, x;
    uint32_t ulTotalTime, ulStatsAsPercentage;
    std::map<UBaseType_t, uint32_t> task_run_time;
    uint32_t last_total_time = 0;
    UBaseType_t filter[8];
    uint32_t run_time;

    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = (TaskStatus_t*)pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );
    uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalTime );
    uint8_t filter_idx = 0;
    for(x = 0; x < uxArraySize; x++ )
    {
        if (strncmp(pxTaskStatusArray[ x ].pcTaskName, "IDLE", 4) == 0)
        {
            filter[filter_idx++] = pxTaskStatusArray[ x ].xTaskNumber;
        }

        else if (strncmp(pxTaskStatusArray[ x ].pcTaskName, "ipc", 3) == 0)
        {
            filter[filter_idx++] = pxTaskStatusArray[ x ].xTaskNumber;
        }

        else if (strncmp(pxTaskStatusArray[ x ].pcTaskName, "main", 4) == 0)
        {
            filter[filter_idx++] = pxTaskStatusArray[ x ].xTaskNumber;
        }

        else if (strncmp(pxTaskStatusArray[ x ].pcTaskName, "esp_", 4) == 0)
        {
            filter[filter_idx++] = pxTaskStatusArray[ x ].xTaskNumber;
        }
    }

    while (1)
    {
        uxArraySize = uxTaskGetNumberOfTasks();
        pxTaskStatusArray = (TaskStatus_t*)pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalTime );
        ulTotalTime /= 100UL;
        if (ulTotalTime == 0)
        {
            continue;
        }

        uint32_t delta = ulTotalTime - last_total_time;
        last_total_time = ulTotalTime;

        InsertionSort(compareUsage, pxTaskStatusArray, uxArraySize);

        for(x = 0; x < uxArraySize; x++ )
        {
            for (uint8_t i = 0; i < filter_idx; i++)
            {
                if (filter[i] == pxTaskStatusArray[ x ].xTaskNumber)
                {
                    goto skip;
                }
            }

            run_time = pxTaskStatusArray[ x ].ulRunTimeCounter;
            if (task_run_time.count(pxTaskStatusArray[ x ].xTaskNumber) != 0)
            {
                run_time = pxTaskStatusArray[ x ].ulRunTimeCounter - task_run_time[pxTaskStatusArray[ x ].xTaskNumber] ;
            }
            task_run_time[pxTaskStatusArray[ x ].xTaskNumber] = pxTaskStatusArray[ x ].ulRunTimeCounter;

            ulStatsAsPercentage = run_time / delta;

            if( ulStatsAsPercentage > 0UL )
            {
                printf("%-3d %-24s %2d %1d %9d %2u%%\n",
                    pxTaskStatusArray[ x ].xTaskNumber, 
                    pxTaskStatusArray[ x ].pcTaskName, 
                    pxTaskStatusArray[ x ].uxCurrentPriority,
                    pxTaskStatusArray[ x ].xCoreID,
                    run_time,
                    ulStatsAsPercentage
                );
            }
            else
            {
                printf("%-3d %-24s %2d %1d %9d <1%%\n", 
                    pxTaskStatusArray[ x ].xTaskNumber,
                    pxTaskStatusArray[ x ].pcTaskName, 
                    pxTaskStatusArray[ x ].uxCurrentPriority,
                    pxTaskStatusArray[ x ].xCoreID,
                    run_time
                );
            }

            skip:;
        }
        vPortFree( pxTaskStatusArray );
        printf("\n");

        vTaskDelay(1000);
    }
}

void startSystemStatsTask()
{
    xTaskCreatePinnedToCore(&System_StatsTask, "System_StatsTask", 4 * 1024, NULL, 15, NULL, 1);
}

void convertToXY(int x0, int y0, float r, float degree, int* x1, int* y1) {
    float radian = degree * (PI / 180.0); // 将度数转换为弧度
    *x1 = x0 + r * cos(radian);
    *y1 = y0 + r * sin(radian);
}

float ease_out_cubic(float x) {
    return 1 - pow(1 - x, 3);
}

float lerp(float start, float end, float t) {
    return start + t * (end - start);
}

bool is_point_inside_circle(int x, int y, int x0, int y0, int r) 
{
    int distance_squared = (x - x0) * (x - x0) + (y - y0) * (y - y0);
    return distance_squared <= r * r;
}

void closest_point_on_circle_edge(int x, int y, int x0, int y0, int r, int *closest_x, int *closest_y) {
    // 计算从圆心到点(x, y)的方向向量
    float dx = x - x0;
    float dy = y - x0;

    // 计算该向量的长度 (距离)
    float distance = sqrtf(dx * dx + dy * dy);

    // 缩放方向向量，使其长度为r
    dx = dx / distance * r;
    dy = dy / distance * r;

    // 计算最近点的坐标
    *closest_x = (int) (x0 + dx);
    *closest_y = (int) (y0 + dy);
}

float update_ema_rms(float ema_rms, float new_rms, float alpha)
{
    ema_rms = (1 - alpha) * ema_rms + alpha * new_rms;
    return ema_rms;
}

float log_map(float a, float a1, float a2, float b1, float b2) {
    if (a1 == 0 || a == 0 || a <= a1) {
        return b1;
    }
    if (a >= a2) {
        return b2;
    }
    return b1 + (b2 - b1) * (log10(a / a1) / log10(a2 / a1));
}


float calculate_rms(int16_t * data, uint32_t len, uint8_t channel)
{
    uint64_t sum = 0;
    for (int i = 0; i < len; i += channel) {
        sum += data[i] * data[i];
    }
    return sqrtf(sum);
}

void generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate, uint8_t channels){

    // See this for reference: http://soundfile.sapp.org/doc/WaveFormat/
    uint32_t file_size = wav_size + WAVE_HEADER_SIZE - 8;
    uint32_t byte_rate = BYTE_RATE;

    const char set_wav_header[] = {
        'R','I','F','F', // ChunkID
        file_size, file_size >> 8, file_size >> 16, file_size >> 24, // ChunkSize
        'W','A','V','E', // Format
        'f','m','t',' ', // Subchunk1ID
        0x10, 0x00, 0x00, 0x00, // Subchunk1Size (16 for PCM)
        0x01, 0x00, // AudioFormat (1 for PCM)
        channels, 0x00, // NumChannels (1 channel)
        sample_rate, sample_rate >> 8, sample_rate >> 16, sample_rate >> 24, // SampleRate
        byte_rate, byte_rate >> 8, byte_rate >> 16, byte_rate >> 24, // ByteRate
        0x02, 0x00, // BlockAlign
        0x10, 0x00, // BitsPerSample (16 bits)
        'd','a','t','a', // Subchunk2ID
        wav_size, wav_size >> 8, wav_size >> 16, wav_size >> 24, // Subchunk2Size
    };

    memcpy(wav_header, set_wav_header, sizeof(set_wav_header));
}

void listFiles(const char * dirName) 
{
  File dir = SD.open(dirName);
  
  if (!dir) {
    printf("Failed to open directory\n");
    return;
  }
  
  if (!dir.isDirectory()) {
    printf("Not a directory\n");
    return;
  }

  File entry =  dir.openNextFile();
  
  while (entry) {
    if (entry.isDirectory()) {
      printf("DIR : %s\n", entry.name());
    } else {
      printf("FILE: %s\t%ld\n", entry.name(), entry.size());
    }
    entry.close();
    entry = dir.openNextFile();
  }
}

inline uint32_t _calc2x2Block(uint16_t* image, uint32_t width, uint32_t height, uint32_t row, uint32_t column)
{
    uint32_t base0 = row * width + column;
    uint32_t base1 = base0 + width;
    return (
        rgb565_to_gray_lut[image[base0]] + 
        rgb565_to_gray_lut[image[base1]] + 
        rgb565_to_gray_lut[image[base0 + 1]] + 
        rgb565_to_gray_lut[image[base1 + 1]]) >> 2;
}

inline void _applyDitheredVal(uint16_t grayscale, uint8_t *line0, uint8_t *line1)
{
    *line0 <<= 2;
    *line1 <<= 2;
    if (grayscale < 84) {
        *line0 |= 0b11;
        *line1 |= 0b11;
    }
    else if (grayscale < 126) {
        *line0 |= 0b10;
        *line1 |= 0b11;
    }
    else if (grayscale < 168) {
        *line0 |= 0b10;
        *line1 |= 0b01;
    }
    else if (grayscale < 210) {
        *line0 |= 0b10;
        *line1 |= 0b00;
    }
    else {
        *line0 |= 0b00;
        *line1 |= 0b00;
    }
}

void histogramEqualization(const uint8_t* input, uint8_t* output, int width, int height) {
    #define LEVELS 256
    int hist[LEVELS] = {0};
    float prob[LEVELS] = {0.0};
    float cdf[LEVELS] = {0.0};

    // 计算直方图
    for (int i = 0; i < width * height; i++) {
        hist[input[i]]++;
    }

    // 计算概率
    for (int i = 0; i < LEVELS; i++) {
        prob[i] = (float)hist[i] / (width * height);
    }

    // 计算累积分布函数 (CDF)
    cdf[0] = prob[0];
    for (int i = 1; i < LEVELS; i++) {
        cdf[i] = cdf[i - 1] + prob[i];
    }

    // 重新映射像素值
    for (int i = 0; i < width * height; i++) {
        output[i] = (uint8_t)(cdf[input[i]] * 255.0);
    }
}


void ditherImg(uint16_t* image, uint8_t* dst, uint32_t width, uint32_t height) 
{
    uint32_t dst_width = width >> 3;
    uint32_t gray_average = 0;
    
    for (uint32_t row = 0; row < height; row += 2) {
        uint32_t base = row * dst_width;
        for (uint32_t column = 0; column < width; column += 8) {
            uint8_t line0 = 0, line1 = 0;
            uint16_t g0 = _calc2x2Block(image, width, height, row, column + 0);
            uint16_t g1 = _calc2x2Block(image, width, height, row, column + 2);
            uint16_t g2 = _calc2x2Block(image, width, height, row, column + 4);
            uint16_t g3 = _calc2x2Block(image, width, height, row, column + 6);

            _applyDitheredVal(g0, &line0, &line1);
            _applyDitheredVal(g1, &line0, &line1);
            _applyDitheredVal(g2, &line0, &line1);
            _applyDitheredVal(g3, &line0, &line1);

            uint32_t idx_0 = base + (column >> 3);
            uint32_t idx_1 = base + (column >> 3) + dst_width;

            dst[idx_0] = line0;
            dst[idx_1] = line1;
        }
    }
}