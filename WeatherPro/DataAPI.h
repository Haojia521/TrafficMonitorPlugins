#pragma once

#include <string>
#include <vector>
#include <memory>

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

class DataAPI
{
public:
    virtual bool QueryCity(const std::wstring &query, CityInfoList &info) = 0;

    virtual std::wstring GetWeatherInfoSummary() = 0;
    virtual std::wstring GetTemprature(EWeatherInfoType type) = 0;
    virtual std::wstring GetWeatherText(EWeatherInfoType type) = 0;
    virtual std::wstring GetWeatherCode(EWeatherInfoType type) = 0;

    virtual bool UpdateWeather() = 0;

    const WStringList& GetErrors() { return m_errors; }
    void ClrearErrors() { m_errors.clear(); }

protected:
    WStringList m_errors;
};

using DataApiPtr = std::shared_ptr<DataAPI>;
