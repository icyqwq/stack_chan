#pragma once

#include "api_base.hpp"


class OpenWeatherAPI: public APIBase
{
public:
    static inline constexpr char* k_languages[NUM_LANGUAGES + 1] = {
        "en","ja","zh_cn","zh_tw", "\0"
    };

    static inline constexpr char* k_aqi_desc_en[] = {
        "\0", "Good", "Fair", "Moderate", "Poor", "Very Poor", "\0"
    };

    static inline constexpr char* k_aqi_desc_cn[] = {
        "\0", "优", "良", "中等", "差", "非常差", "\0"
    };

    static inline constexpr char* k_aqi_desc_ja[] = {
        "\0", "良好", "普通", "適度", "悪い", "非常に悪い", "\0"
    };

private:
    static inline constexpr std::pair<int, skycon_code_t> _skycon_map[]  = {
        {200, SKYCON_CODE_THUNDERSTORM_LIGHT_RAIN},
        {201, SKYCON_CODE_THUNDERSTORM_MODERATE_RAIN},
        {202, SKYCON_CODE_THUNDERSTORM_HEAVY_RAIN},
        {210, SKYCON_CODE_THUNDERSTORM},
        {211, SKYCON_CODE_THUNDERSTORM},
        {212, SKYCON_CODE_THUNDERSTORM},
        {221, SKYCON_CODE_THUNDERSTORM},
        {230, SKYCON_CODE_THUNDERSTORM_LIGHT_RAIN},
        {231, SKYCON_CODE_THUNDERSTORM_MODERATE_RAIN},
        {232, SKYCON_CODE_THUNDERSTORM_HEAVY_RAIN},

        {300, SKYCON_CODE_LIGHT_RAIN},
        {301, SKYCON_CODE_MODERATE_RAIN},
        {302, SKYCON_CODE_HEAVY_RAIN},
        {310, SKYCON_CODE_LIGHT_RAIN},
        {311, SKYCON_CODE_MODERATE_RAIN},
        {312, SKYCON_CODE_HEAVY_RAIN},
        {313, SKYCON_CODE_MODERATE_RAIN},
        {314, SKYCON_CODE_HEAVY_RAIN},
        {321, SKYCON_CODE_LIGHT_RAIN},

        {500, SKYCON_CODE_LIGHT_RAIN},
        {501, SKYCON_CODE_MODERATE_RAIN},
        {502, SKYCON_CODE_HEAVY_RAIN},
        {503, SKYCON_CODE_STORM_RAIN},
        {504, SKYCON_CODE_STORM_RAIN},
        {511, SKYCON_CODE_LIGHT_RAIN},
        {520, SKYCON_CODE_LIGHT_RAIN},
        {521, SKYCON_CODE_MODERATE_RAIN},
        {522, SKYCON_CODE_HEAVY_RAIN},
        {531, SKYCON_CODE_STORM_RAIN},

        {600, SKYCON_CODE_LIGHT_SNOW},
        {601, SKYCON_CODE_MODERATE_SNOW},
        {602, SKYCON_CODE_HEAVY_SNOW},
        {611, SKYCON_CODE_LIGHT_SNOW},
        {612, SKYCON_CODE_LIGHT_SNOW},
        {613, SKYCON_CODE_LIGHT_SNOW},
        {615, SKYCON_CODE_LIGHT_SNOW},
        {616, SKYCON_CODE_LIGHT_SNOW},
        {620, SKYCON_CODE_LIGHT_SNOW},
        {621, SKYCON_CODE_MODERATE_SNOW},
        {622, SKYCON_CODE_HEAVY_SNOW},

        {701, SKYCON_CODE_LIGHT_HAZE},
        {711, SKYCON_CODE_MODERATE_HAZE},
        {721, SKYCON_CODE_HEAVY_HAZE},
        {731, SKYCON_CODE_LIGHT_HAZE},
        {741, SKYCON_CODE_MODERATE_HAZE},
        {751, SKYCON_CODE_HEAVY_HAZE},
        {761, SKYCON_CODE_HEAVY_HAZE},
        {762, SKYCON_CODE_HEAVY_HAZE},
        {771, SKYCON_CODE_WIND},
        {781, SKYCON_CODE_WIND},

        {800, SKYCON_CODE_CLEAR_DAY},

        {801, SKYCON_CODE_PARTLY_CLOUDY_DAY},
        {802, SKYCON_CODE_PARTLY_CLOUDY_DAY},
        {803, SKYCON_CODE_CLOUDY},
        {804, SKYCON_CODE_CLOUDY},

        {0, SKYCON_CODE_NA},
    };

    static constexpr char* API_URL = "https://api.openweathermap.org/data/3.0/onecall";
    static constexpr char* AQI_API_URL = "http://api.openweathermap.org/data/2.5/air_pollution";
    JsonVariant alert;
    JsonVariant realtime;
    JsonVariant minutely;
    JsonVariant hourly;
    JsonVariant daily;
    String _aqi_url;
    bool _has_alert = false;
    bool _has_minutely = false;
    int _aqi_level;

public:
    OpenWeatherAPI()
    {
        _aqi_level = 0;
        _lang_code = LANGUAGE_ENGLISH;
        _language = k_languages[_lang_code];
    }

    ~OpenWeatherAPI() {}

    void setLanguage(int lang) override
    {
        if (lang >= NUM_LANGUAGES) {
            lang = LANGUAGE_ENGLISH;
        }
        _lang_code = lang;
        _language = k_languages[lang];
    }

    void updateURL() override
    {
        //https://api.openweathermap.org/data/3.0/onecall?lat=0.1276&lon=51.5072&appid=097a18e6638a79a1f036b135d4b4a4b4&units=metric&lang=zh_cn
        char buf[256];
        sprintf(buf, "%s?lat=%s&lon=%s&appid=%s&units=metric&lang=%s", API_URL, _latitude.c_str(), _longitude.c_str(), _token.c_str(), _language.c_str());
        _url = buf;
        sprintf(buf, "%s?lat=%s&lon=%s&appid=%s&units=metric", AQI_API_URL, _latitude.c_str(), _longitude.c_str(), _token.c_str());
        _aqi_url = buf;
        ESP_LOGI(TAG, "API URL: %s", _url.c_str());
        ESP_LOGI(TAG, "AQI URL: %s", _aqi_url.c_str());
    }

    template <typename T>
    std::vector<T> _convToVector(JsonVariant &ref, String key, const std::function<T(JsonVariant&)>& convertItem = nullptr)
    {
        JsonArray precipitationArray = ref.as<JsonArray>();
        std::vector<T> vec;
        vec.reserve(precipitationArray.size());
        
        for(JsonVariant v : precipitationArray) {
            if (convertItem) {
                JsonVariant val = v[key];
                vec.emplace_back(convertItem(val));
            } else {
                vec.emplace_back(v[key].as<T>());
            }
        }
        return vec;
    }

    esp_err_t request() override
    {
        if (requestOneCall() != ESP_OK) {
            return ESP_FAIL;
        }
        if (requestAQI() != ESP_OK) {
            return ESP_FAIL;
        }
        return ESP_OK;
    }

    esp_err_t requestOneCall()
    {
        int retry_cnt = 0;
        int httpCode;
        _err_str = "OK";
        _has_alert = false;
        _has_minutely = false;

        while (1)
        {
            HTTPClient http;
            http.begin(_url);
            httpCode = http.GET();

            if (httpCode == 200)
            {
                doc->clear();
                DeserializationError err = deserializeJson(*doc, http.getString());
                if (err)
                {
                    _err_str = String("JSON Parse Faild, ") + err.c_str();
                    log_e("deserializeJson failed: %s", err.c_str());
                }
                else if((*doc).containsKey("cod"))
                {
                    _err_str = "API Status ERROR: " + (*doc)["cod"].as<int>();
                    http.end();
                    return ESP_FAIL;
                }
                else
                {
                    if ((*doc).containsKey("alerts")) {
                        _has_alert = true;
                        alert = (*doc)["alerts"];
                    }

                    if ((*doc).containsKey("minutely")) {
                        _has_minutely = true;
                        minutely = (*doc)["minutely"];
                    }
                    
                    realtime = (*doc)["current"];
                    hourly = (*doc)["hourly"];
                    daily = (*doc)["daily"];
                    ESP_LOGI(TAG, "Success");
                    http.end();
                    break;
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

    esp_err_t requestAQI()
    {
        int retry_cnt = 0;
        int httpCode;
        _err_str = "OK";

        while (1)
        {
            HTTPClient http;
            http.begin(_aqi_url);
            httpCode = http.GET();

            if (httpCode == 200)
            {
                DynamicJsonDocument doc(2048);
                DeserializationError err = deserializeJson(doc, http.getString());
                if (err)
                {
                    _err_str = String("JSON Parse Faild, ") + err.c_str();
                    log_e("deserializeJson failed: %s", err.c_str());
                }
                else if(doc.containsKey("cod"))
                {
                    _err_str = "API Status ERROR: " + doc["cod"].as<int>();
                    http.end();
                    return ESP_FAIL;
                }
                else
                {
                    _aqi_level = doc["list"][0]["main"]["aqi"];
                    http.end();
                    break;
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
        setToken("097a18e6638a79a1f036b135d4b4a4b4");
        setCoordinate("113.82353", "22.66479");
        setLanguage(LANGUAGE_CHINESE_SIMPLIFIED);
        updateURL();
        request();

        auto funcPrintString = [](const String& elem) { printf("%s", elem.c_str()); };
        auto funcPrintFloat = [](const float& elem) { printf("%f", elem); };
        auto funcPrintSkycon = [](const skycon_code_t& elem) { printf("%d", elem); };
        auto funcPrintTimeT = [](const time_t& elem) { printf("%d", elem); };

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
        return realtime["dt"];
    }

    String getKeypoint() override
    {
        return daily[0]["summary"];
    }

    std::vector<String> getAlert() override
    {
        if (!_has_alert) {
            return std::vector<String>();
        }

        return _convToVector<String>(alert, "event");
    }

    float getRealtimeTemperature() override
    {
        return realtime["temp"];
    }

    skycon_code_t getRealtimeSkycon() override
    {
        return _map_find(realtime["weather"][0]["id"].as<int>(), this->_skycon_map);
    }

    String getRealtimeSkyconDescription() override
    {
        return realtime["weather"][0]["description"];
    }

    float getRealtimeWindSpeed() override
    {
        return realtime["wind_speed"];
    }

    float getRealtimeWindDirection() override
    {
        return realtime["wind_deg"];
    }

    float getRealtimePrecipitationIntensity() override
    {
        return 0;
    }

    int getRealtimeAirQualityAQI() override
    {
        return 0;
    }

    String getRealtimeAirQualityDescription() override
    {
        switch (_lang_code)
        {
        case LANGUAGE_JAPANESE: return k_aqi_desc_ja[_aqi_level];
        case LANGUAGE_CHINESE_SIMPLIFIED: 
        case LANGUAGE_CHINESE_TRADITIONAL: return k_aqi_desc_cn[_aqi_level];
        
        default: return k_aqi_desc_en[_aqi_level];
        }
    }

    std::vector<float> getMinutelyPrecipitation() override
    {
        if (!_has_minutely) {
            return std::vector<float>();
        }
        return _convToVector<float>(minutely, "precipitation");
    }

    std::vector<float> getMinutelyProbability() override
    {
        return std::vector<float>();
    }

    std::vector<float> getHourlyPrecipitation() override
    {
        // Probability of precipitation. The values of the parameter vary between 0 and 1, where 0 is equal to 0%, 1 is equal to 100%
        return _convToVector<float>(hourly, "pop");
    }

    std::vector<float> getHourlyTemperature() override
    {
        return _convToVector<float>(hourly, "temp", [this](JsonVariant &elem) -> float {
                return elem.as<float>();
        });
    }

    std::vector<float> getHourlyWindSpeed() override
    {
        return _convToVector<float>(hourly, "wind_speed");
    }

    std::vector<skycon_code_t> getHourlySkycon() override
    {
        return _convToVector<skycon_code_t>(hourly, "weather", [this](JsonVariant &elem) -> skycon_code_t {
                return _map_find(elem[0]["id"].as<int>(), this->_skycon_map);
            });
    }

    float getTodayTemperatureMax() override
    {
        return daily[0]["temp"]["max"];
    }

    float getTodayTemperatureMin() override
    {
        return daily[0]["temp"]["min"];
    }

    std::vector<time_t> getSunTime() override
    {
        JsonArray precipitationArray = daily.as<JsonArray>();
        std::vector<time_t> vec;
        vec.reserve(precipitationArray.size() * 2);
        
        for(JsonVariant v : precipitationArray) {
            vec.emplace_back(v["sunrise"].as<time_t>());
            vec.emplace_back(v["sunset"].as<time_t>());
        }
        return vec;
    }
};

