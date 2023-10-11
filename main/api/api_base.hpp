#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ctime>

#include <vector>
#include <algorithm>
#include <utility>
#include <cstring> 
#include "StreamSPIString.hpp"

struct SPIRAMAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};

using SPIRAMJsonDocument = BasicJsonDocument<SPIRAMAllocator>;

class APIBase
{
public:
    typedef enum {
        SKYCON_CODE_NA = 0,
        SKYCON_CODE_CLEAR_DAY,
        SKYCON_CODE_CLEAR_NIGHT,
        SKYCON_CODE_PARTLY_CLOUDY_DAY,
        SKYCON_CODE_PARTLY_CLOUDY_NIGHT,
        SKYCON_CODE_CLOUDY,
        SKYCON_CODE_LIGHT_HAZE,
        SKYCON_CODE_MODERATE_HAZE,
        SKYCON_CODE_HEAVY_HAZE,
        SKYCON_CODE_LIGHT_RAIN,
        SKYCON_CODE_MODERATE_RAIN,
        SKYCON_CODE_HEAVY_RAIN,
        SKYCON_CODE_STORM_RAIN,
        SKYCON_CODE_THUNDERSTORM,
        SKYCON_CODE_THUNDERSTORM_LIGHT_RAIN,
        SKYCON_CODE_THUNDERSTORM_MODERATE_RAIN,
        SKYCON_CODE_THUNDERSTORM_HEAVY_RAIN,
        SKYCON_CODE_THUNDERSTORM_STORM_RAIN,
        SKYCON_CODE_FOG,
        SKYCON_CODE_LIGHT_SNOW,
        SKYCON_CODE_MODERATE_SNOW,
        SKYCON_CODE_HEAVY_SNOW,
        SKYCON_CODE_STORM_SNOW,
        SKYCON_CODE_DUST,
        SKYCON_CODE_SAND,
        SKYCON_CODE_WIND,
    } skycon_code_t;

    typedef enum {
        LANGUAGE_ENGLISH,
        LANGUAGE_JAPANESE,
        LANGUAGE_CHINESE_SIMPLIFIED,
        LANGUAGE_CHINESE_TRADITIONAL,
        NUM_LANGUAGES,
        LANGUAGE_DEFAULT,
    } language_code_t;

protected:
    static constexpr char* TAG = "API";
    String _err_str;
    String _token;
    String _longitude;
    String _latitude;
    int _lang_code;
    String _url;
    SPIRAMJsonDocument *doc;

protected:
    template <std::size_t N>
    static skycon_code_t _map_find(const char* key, const std::pair<const char*, skycon_code_t> (&map)[N]) {
        auto it = std::lower_bound(
            std::begin(map), std::end(map), key,
            [](const std::pair<const char*, skycon_code_t>& pair, const char* key) { return std::strcmp(pair.first, key) < 0; }
        );
        if (it != std::end(map) && std::strcmp(it->first, key) == 0)
            return it->second;
        return SKYCON_CODE_NA;
    }

    template <std::size_t N>
    static skycon_code_t _map_find(int key, const std::pair<int, skycon_code_t> (&map)[N]) {
        auto it = std::lower_bound(
            std::begin(map), std::end(map), key,
            [](const std::pair<int, skycon_code_t>& pair, int key) { return pair.first < key; }
        );
        if (it != std::end(map) && it->first == key)
            return it->second;
        return SKYCON_CODE_NA;
    }

public:
    APIBase(/* args */) 
    {
        doc = new SPIRAMJsonDocument(64 * 1024);
    }
    ~APIBase() 
    {
        delete doc;
    }

    template <typename T>
    static void printVector(String prepend, const std::vector<T> vec, const std::function<void(const T&)>& printElement) {
        printf("%s: [", prepend.c_str());
        for(const auto& elem : vec) {
            printElement(elem);
            printf(",");
        }
        printf("]\n");
    }

    static float kelvin_to_celsius(float kelvin) {
        return kelvin - 273.15f;
    }

    static float kelvin_to_fahrenheit(float kelvin) {
        return ((kelvin - 273.15) * 9.0 / 5.0) + 32.0;
    }

    static float celsius_to_fahrenheit(float celsius) {
        return (celsius * 9 / 5) + 32;
    }

    String& getErrMsg()
    {
        return _err_str;
    }

    void setToken(String token) 
    {
        _token = token;    
    }

    void setCoordinate(String longitude, String latitude)
    {
        _longitude = longitude;
        _latitude = latitude;
    }

    int getLanguageCode()
    {
        return _lang_code;
    }

    void setLanguage(int lang)
    {
        if (lang >= NUM_LANGUAGES) {
            lang = LANGUAGE_ENGLISH;
        }
        _lang_code = lang;
    }

    virtual void updateURL() = 0;
    virtual esp_err_t request() = 0;

    // Timestamp
    virtual uint64_t getServerTime() = 0;

    // Weather condition summary
    virtual String getKeypoint() = 0;

    // Alert issued by local weather bureau 
    virtual std::vector<String> getAlert() = 0;

    // Current temperature
    virtual float getRealtimeTemperature() = 0;

    // Current skycon string, on of skycon_code_t
    virtual skycon_code_t getRealtimeSkycon() = 0;

    // Description for sky condition, like: Clouds
    virtual String getRealtimeSkyconDescription(language_code_t lang = LANGUAGE_DEFAULT) = 0;

    // m/s
    virtual float getRealtimeWindSpeed() = 0;

    // Meteorological degrees
    virtual float getRealtimeWindDirection() = 0;

    // meter/sec
    virtual float getRealtimePrecipitationIntensity() = 0;

    // Current AQI
    virtual int getRealtimeAirQualityAQI() = 0;

    // eg: good, bad
    virtual String getRealtimeAirQualityDescription() = 0;

    // Minutely precipitation array. mm/hr
    // [0.0, 0.0, 0.1, ..., 1.1] One minute for each item.
    virtual std::vector<float> getMinutelyPrecipitation() = 0;

    // Minutely precipitation probability
    // [max,min,avg,probability]
    virtual std::vector<float> getMinutelyProbability() = 0;

    // Hourly precipitation array. // mm/hr
    // [0.0, 0.0, 0.1, ..., 1.1] One hour for each item.
    virtual std::vector<float> getHourlyPrecipitation() = 0;

    // Hourly temperature array, One hour for each item.
    virtual std::vector<float> getHourlyTemperature() = 0;

    // Hourly wind speed array, One hour for each item.
    virtual std::vector<float> getHourlyWindSpeed() = 0;

    // Hourly skycon array, One hour for each item. Same as getRealtimeSkycon
    virtual std::vector<skycon_code_t> getHourlySkycon() = 0;

    // Today's max temperature
    virtual float getTodayTemperatureMax() = 0;

    // Today's min temperature
    virtual float getTodayTemperatureMin() = 0;

    // [day 1 sunrise, day 1 sunset, day 2 sunrise, day 2 sunset] in timestamp
    virtual std::vector<time_t> getSunTime() = 0;
};
