#pragma once

#include <string>
#include <vector>

enum class EWeatherInfoType
{
    WEATHER_REALTIME,
    WEATHER_TODAY,
    WEATHER_TOMMROW,
    WEATHER_DAY_AFTER_TOMMROW
};

struct SCityInfo
{
    std::wstring CityNO;
    std::wstring CityName;
    std::wstring CityAdministrativeOwnership;
};

using CityInfoList = std::vector<SCityInfo>;

class DataAPI
{
public:
    virtual CityInfoList QueryCity(const std::wstring &query) = 0;

    virtual std::wstring GetWeatherInfoSummary() = 0;
    virtual std::wstring GetTemprature(EWeatherInfoType type) = 0;
    virtual std::wstring GetWeatherText(EWeatherInfoType type) = 0;
    virtual std::wstring GetWeatherCode(EWeatherInfoType type) = 0;

    virtual bool UpdateWeather() = 0;

    virtual std::wstring GetLastError() = 0;
};
