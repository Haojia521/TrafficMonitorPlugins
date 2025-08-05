#pragma once

#include <string>
#include <vector>
#include <memory>
#include <bitset>

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
using WStringList = std::vector<std::wstring>;
using UpdatingMask = std::bitset<8>;

class DataAPI
{
public:
    virtual bool QueryCity(const std::wstring &query, CityInfoList &info, WStringList &errors) = 0;

    virtual std::wstring GetWeatherInfoSummary(WStringList &errors) = 0;
    virtual std::wstring GetTemprature(EWeatherInfoType type) = 0;
    virtual std::wstring GetWeatherText(EWeatherInfoType type) = 0;
    virtual std::wstring GetWeatherCode(EWeatherInfoType type) = 0;

    virtual bool UpdateWeather(WStringList &errors, UpdatingMask &mask) = 0;

    virtual std::wstring GetHumidity(EWeatherInfoType type) = 0;
    virtual std::wstring GetWind(EWeatherInfoType type) = 0;
    virtual std::wstring GetAQI(EWeatherInfoType type) = 0;
    virtual std::wstring GetPM2p5(EWeatherInfoType type) = 0;
    virtual std::wstring GetPM10(EWeatherInfoType type) = 0;
};

using DataApiPtr = std::shared_ptr<DataAPI>;
