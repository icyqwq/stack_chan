#pragma once

#include "api_base.hpp"


class CaiyunAPI: public APIBase
{
public:
    static inline constexpr char* k_languages[NUM_LANGUAGES + 1] = {
        "en_US", // 美式英语
        "ja", // 日语
        "zh_CN", // 简体中文
        "zh_TW", // 繁体中文
        "\0"
    };

private:
    static inline constexpr std::pair<const char*, skycon_code_t> _skycon_map[]  = {
        {"CLEAR_DAY", SKYCON_CODE_CLEAR_DAY},
        {"CLEAR_NIGHT", SKYCON_CODE_CLEAR_NIGHT},
        {"CLOUDY", SKYCON_CODE_CLOUDY},
        {"DUST", SKYCON_CODE_DUST},
        {"FOG", SKYCON_CODE_FOG},
        {"HEAVY_HAZE", SKYCON_CODE_HEAVY_HAZE},
        {"HEAVY_RAIN", SKYCON_CODE_HEAVY_RAIN},
        {"HEAVY_SNOW", SKYCON_CODE_HEAVY_SNOW},
        {"LIGHT_HAZE", SKYCON_CODE_LIGHT_HAZE},
        {"LIGHT_RAIN", SKYCON_CODE_LIGHT_RAIN},
        {"LIGHT_SNOW", SKYCON_CODE_LIGHT_SNOW},
        {"MODERATE_HAZE", SKYCON_CODE_MODERATE_HAZE},
        {"MODERATE_RAIN", SKYCON_CODE_MODERATE_RAIN},
        {"MODERATE_SNOW", SKYCON_CODE_MODERATE_SNOW},
        {"PARTLY_CLOUDY_DAY", SKYCON_CODE_PARTLY_CLOUDY_DAY},
        {"PARTLY_CLOUDY_NIGHT", SKYCON_CODE_PARTLY_CLOUDY_NIGHT},
        {"SAND", SKYCON_CODE_SAND},
        {"STORM_RAIN", SKYCON_CODE_STORM_RAIN},
        {"STORM_SNOW", SKYCON_CODE_STORM_SNOW},
        {"WIND", SKYCON_CODE_WIND},
        {"null", SKYCON_CODE_NA},
    };

    static inline constexpr char *k_alert_type[18] = {
        "\0", "台风", "暴雨", "暴雪", "寒潮", "大风", "沙尘暴", "高温", "干旱", "雷电", "冰雹", "霜冻", "大雾", "霾", "道路结冰", "森林火灾", "雷雨大风", "\0"};

    static inline constexpr char *k_alert_level[6] = {
        "\0", "I", "II", "III", "IV", "\0"};

    static inline constexpr char *k_skycon_desc_cn[] = {
        "未知",   // 0
        "晴",     // 1 CLEAR_DAY
        "晴夜",   // 2 CLEAR_NIGHT
        "多云",   // 3 PARTLY_CLOUDY_DAY
        "云夜",   // 4 PARTLY_CLOUDY_NIGHT
        "阴",     // 5 CLOUDY
        "雾霾",   // 6 LIGHT_HAZE
        "中雾霾", // 7 MODERATE_HAZE
        "重雾霾", // 8 HEAVY_HAZE
        "小雨",   // 9 LIGHT_RAIN
        "中雨",   // 10 MODERATE_RAIN
        "大雨",   // 11 HEAVY_RAIN
        "暴雨",   // 12 STORM_RAIN TODO: STORM_RAIN ICON
        "雾",     // 13 FOG  TODO: FOG ICON
        "小雪",   // 14 LIGHT_SNOW
        "中雪",   // 15 MODERATE_SNOW
        "大雪",   // 16 HEAVY_SNOW
        "暴雪",   // 17 STORM_SNOW  TODO: STORM_SNOW ICON
        "扬尘",   // 18 DUST  TODO: DUST ICON
        "沙尘",   // 19 SAND  TODO: SAND ICON
        "大风",   // 20 WIND
        "\0"};

    static inline constexpr char *k_skycon_desc_ja[] = {
        "未知",   // 0
        "晴れ",     // 1 CLEAR_DAY
        "晴夜",   // 2 CLEAR_NIGHT
        "薄曇り",   // 3 PARTLY_CLOUDY_DAY
        "薄曇り",   // 4 PARTLY_CLOUDY_NIGHT
        "曇",     // 5 CLOUDY
        "煙霧",   // 6 LIGHT_HAZE
        "煙霧", // 7 MODERATE_HAZE
        "煙霧", // 8 HEAVY_HAZE
        "小雨",   // 9 LIGHT_RAIN
        "弱い雨",   // 10 MODERATE_RAIN
        "強い雨",   // 11 HEAVY_RAIN
        "大雨",   // 12 STORM_RAIN TODO: STORM_RAIN ICON
        "霧",     // 13 FOG  TODO: FOG ICON
        "小雪",   // 14 LIGHT_SNOW
        "弱い雪",   // 15 MODERATE_SNOW
        "強い雪",   // 16 HEAVY_SNOW
        "大雪",   // 17 STORM_SNOW  TODO: STORM_SNOW ICON
        "砂塵",   // 18 DUST  TODO: DUST ICON
        "砂嵐",   // 19 SAND  TODO: SAND ICON
        "強風",   // 20 WIND
        "\0"};

    static inline constexpr char *k_skycon_desc_en[] = {
        "Unknown",   // 0
        "Clear",     // 1 CLEAR_DAY
        "Clear",   // 2 CLEAR_NIGHT
        "Cloudy",   // 3 PARTLY_CLOUDY_DAY
        "Cloudy",   // 4 PARTLY_CLOUDY_NIGHT
        "Cloudy",     // 5 CLOUDY
        "Haze",   // 6 LIGHT_HAZE
        "Haze", // 7 MODERATE_HAZE
        "Haze", // 8 HEAVY_HAZE
        "Rain",   // 9 LIGHT_RAIN
        "Rain",   // 10 MODERATE_RAIN
        "Rain",   // 11 HEAVY_RAIN
        "Rain",   // 12 STORM_RAIN TODO: STORM_RAIN ICON
        "Fog",     // 13 FOG  TODO: FOG ICON
        "Snow",   // 14 LIGHT_SNOW
        "Snow",   // 15 MODERATE_SNOW
        "Snow",   // 16 HEAVY_SNOW
        "Snow",   // 17 STORM_SNOW  TODO: STORM_SNOW ICON
        "Dust",   // 18 DUST  TODO: DUST ICON
        "Sand",   // 19 SAND  TODO: SAND ICON
        "Wind",   // 20 WIND
        "\0"};

    static constexpr char* API_URL = "https://api.caiyunapp.com/v2.6";
    JsonVariant alert;
    JsonVariant realtime;
    JsonVariant minutely;
    JsonVariant hourly;
    JsonVariant daily;

public:
    CaiyunAPI()
    {
        _lang_code = LANGUAGE_ENGLISH;
    }

    ~CaiyunAPI() {}

    void updateURL() override
    {
        char buf[256];
        sprintf(buf, "%s/%s/%s,%s/weather?alert=true&dailysteps=2&hourlysteps=1&lang=%s&unit=metric:v2", API_URL, _token.c_str(), _longitude.c_str(), _latitude.c_str(),  k_languages[_lang_code]);
        _url = buf;
        ESP_LOGI(TAG, "API URL: %s", buf);
    }

    template <typename T>
    std::vector<T> _convToVector(JsonVariant &ref, String key, String subkey = "value", const std::function<T(JsonVariant&)>& convertItem = nullptr)
    {
        JsonArray precipitationArray = ref[key].as<JsonArray>();
        std::vector<T> vec;
        vec.reserve(precipitationArray.size());
        
        for(JsonVariant v : precipitationArray) {
            if (convertItem) {
                JsonVariant val = v[subkey];
                vec.emplace_back(convertItem(val));
            } else {
                vec.emplace_back(v[subkey].as<T>());
            }
        }
        return vec;
    }

    esp_err_t request() override
    {
        int retry_cnt = 0;
        int httpCode;
        _err_str = "OK";

        while (1)
        {
            HTTPClient http;
            http.begin(_url);
            httpCode = http.GET();

            if (httpCode == 200)
            {
                doc->clear();
                StreamSPIString response;
                http.writeToStream(&response);
                DeserializationError err = deserializeJson(*doc, response.c_str());
                if (err)
                {
                    _err_str = String("JSON Parse Faild, ") + err.c_str();
                    log_e("deserializeJson failed: %s", err.c_str());
                }
                else if((*doc)["status"].as<String>() == "ok")
                {
                    alert = (*doc)["result"]["alert"];
                    realtime = (*doc)["result"]["realtime"];
                    minutely = (*doc)["result"]["minutely"];
                    hourly = (*doc)["result"]["hourly"];
                    daily = (*doc)["result"]["daily"];
                    ESP_LOGI(TAG, "Success");
                    http.end();
                    break;
                }
                else
                {
                    _err_str = "API Status ERROR: " + (*doc)["error"].as<String>();
                    http.end();
                    return ESP_FAIL;
                }
            }
            else
            {
                _err_str = "HTTP ERROR, Code: " + String(httpCode);
            }

            retry_cnt++;
            ESP_LOGW(TAG, "Code = %d, Retry %d", httpCode, retry_cnt);
            if (retry_cnt >= 3)
            {
                _err_str = "Timeout";
                http.end();
                return ESP_FAIL;
            }
            delay(1000);
        }

        ESP_LOGI(TAG, "http code = %d\n", httpCode);
        return ESP_OK;
    }

    void test()
    {
        setToken("OFP0WPweUhyhZ539");
        setCoordinate("113.82353", "22.66479");
        setLanguage(LANGUAGE_CHINESE_SIMPLIFIED);
        updateURL();
        request();

        auto funcPrintString = [](const String& elem) { printf("%s", elem.c_str()); };
        auto funcPrintFloat = [](const float& elem) { printf("%f", elem); };
        auto funcPrintSkycon = [](const skycon_code_t& elem) { printf("%d", elem); };
        auto funcPrintTimeT = [](const time_t& elem) { printf("%lu", elem); };

        printf("Server Time: %llu\n", getServerTime());
        printf("Keypoint: %s\n", getKeypoint().c_str());
        printVector<String>("alert", getAlert(), funcPrintString);

        printf("Realtime Temperature: %f\n", getRealtimeTemperature());
        printf("Realtime Skycon: %d\n", getRealtimeSkycon());
        printf("Realtime Skycon Description: %s\n", getRealtimeSkyconDescription().c_str());
        printf("Realtime Wind Speed: %f\n", getRealtimeWindSpeed());
        printf("Realtime Wind Direction: %f\n", getRealtimeWindDirection());
        printf("Realtime Precipitation Intensity: %f\n", getRealtimePrecipitationIntensity());
        printf("Realtime AQI: %d\n", getRealtimeAirQualityAQI());
        printf("Realtime Air Quality Description: %s\n", getRealtimeAirQualityDescription().c_str());

        printVector<float>("Minutely Precipitation", getMinutelyPrecipitation(), funcPrintFloat);
        printVector<float>("Minutely Probability", getMinutelyProbability(), funcPrintFloat);
        printVector<float>("Hourly Precipitation", getHourlyPrecipitation(), funcPrintFloat);
        printVector<float>("Hourly Temperature", getHourlyTemperature(), funcPrintFloat);
        printVector<float>("Hourly Wind Speed", getHourlyWindSpeed(), funcPrintFloat);
        printVector<skycon_code_t>("Hourly Skycon", getHourlySkycon(), funcPrintSkycon);

        printf("Today Temperature Max: %f\n", getTodayTemperatureMax());
        printf("Today Temperature Min: %f\n", getTodayTemperatureMin());
        printVector<time_t>("Sun rise / set", getSunTime(), funcPrintTimeT);
    }

    uint64_t getServerTime() override
    {
        return (*doc)["server_time"];
    }

    String getKeypoint() override
    {
        return (*doc)["result"]["forecast_keypoint"];
    }

    std::vector<String> getAlert() override
    {
        if (_lang_code != LANGUAGE_CHINESE_SIMPLIFIED) { // for caiyun api, alert only avaliable in china
            return std::vector<String>();
        }
        
        return _convToVector<String>(alert, "content", "code", [this](JsonVariant &elem) -> String {
            String code = elem.as<String>();
            int type = code.substring(0, 2).toInt();
            int level = code.substring(2).toInt();
            return String(k_alert_type[type]) + k_alert_level[level];
        });
    }

    float getRealtimeTemperature() override
    {
        return realtime["temperature"];
    }

    skycon_code_t getRealtimeSkycon() override
    {
        return _map_find(realtime["skycon"].as<String>().c_str(), _skycon_map);
    }

    String getRealtimeSkyconDescription(language_code_t lang = LANGUAGE_DEFAULT) override
    {
        int code = _lang_code;
        if (lang != LANGUAGE_DEFAULT) {
            code = lang;
        }
        const char *const * skycon_desc_table;
        switch (code)
        {
        case LANGUAGE_CHINESE_SIMPLIFIED:
        case LANGUAGE_CHINESE_TRADITIONAL: skycon_desc_table = k_skycon_desc_cn; break; 
        case LANGUAGE_JAPANESE: skycon_desc_table = k_skycon_desc_ja; break;
        default: skycon_desc_table = k_skycon_desc_en; break;
        }
        return skycon_desc_table[getRealtimeSkycon()];
    }

    float getRealtimeWindSpeed() override
    {
        return realtime["speed"];
    }

    float getRealtimeWindDirection() override
    {
        return realtime["direction"];
    }

    float getRealtimePrecipitationIntensity() override
    {
        return realtime["precipitation"]["local"]["intensity"];
    }

    int getRealtimeAirQualityAQI() override
    {
        if (_lang_code == LANGUAGE_CHINESE_SIMPLIFIED) {
            return realtime["air_quality"]["aqi"]["chn"];
        }
        return realtime["air_quality"]["aqi"]["usa"];
    }

    String getRealtimeAirQualityDescription() override
    {
        if (_lang_code == LANGUAGE_CHINESE_SIMPLIFIED) {
            return realtime["air_quality"]["description"]["chn"].as<String>();
        }
        return realtime["air_quality"]["description"]["usa"].as<String>();
    }

    std::vector<float> getMinutelyPrecipitation() override
    {
        return _convToVector<float>(minutely, "precipitation_2h");
    }

    std::vector<float> getMinutelyProbability() override
    {
        return _convToVector<float>(minutely, "probability");
    }

    std::vector<float> getHourlyPrecipitation() override
    {
        return _convToVector<float>(hourly, "precipitation");
    }

    std::vector<float> getHourlyTemperature() override
    {
        return _convToVector<float>(hourly, "temperature");
    }

    std::vector<float> getHourlyWindSpeed() override
    {
        return _convToVector<float>(hourly, "wind", "speed");
    }

    std::vector<skycon_code_t> getHourlySkycon() override
    {
        return _convToVector<skycon_code_t>(hourly, "skycon", "value", [this](JsonVariant &elem) -> skycon_code_t {
                return _map_find(elem.as<String>().c_str(), this->_skycon_map);
            });
    }

    float getTodayTemperatureMax() override
    {
        return daily["temperature"][0]["max"];
    }

    float getTodayTemperatureMin() override
    {
        return daily["temperature"][0]["min"];
    }

    std::vector<time_t> getSunTime()
    {
        JsonArray precipitationArray = daily["astro"].as<JsonArray>();
        std::vector<time_t> vec;
        vec.reserve(precipitationArray.size() * 2);
        
        for(JsonVariant v : precipitationArray) {
            const char* date = v["date"].as<const char*>();
            const char* sunrise = v["sunrise"]["time"].as<const char*>();
            const char* sunset = v["sunset"]["time"].as<const char*>();

            struct tm date_tm = {0};
            strptime(date, "%Y-%m-%dT%H:%M", &date_tm);

            struct tm sunrise_tm = date_tm;
            sscanf(sunrise, "%d:%d", &sunrise_tm.tm_hour, &sunrise_tm.tm_min);
            time_t sunrise_timestamp = mktime(&sunrise_tm);

            struct tm sunset_tm = date_tm;
            sscanf(sunset, "%d:%d", &sunset_tm.tm_hour, &sunset_tm.tm_min);
            time_t sunset_timestamp = mktime(&sunset_tm);

            // printf("Sunrise Timestamp: %ld\n", sunrise_timestamp);
            // printf("Sunset Timestamp: %ld\n", sunset_timestamp);

            vec.emplace_back(sunrise_timestamp);
            vec.emplace_back(sunset_timestamp);
        }
        return vec;
    }
};

