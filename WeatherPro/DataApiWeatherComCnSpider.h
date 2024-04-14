#pragma once
#include "DataAPI.h"

class DataApiWeatherComCnSpider : public DataAPI
{
public:

    struct SRealTimeWeather
    {
        std::wstring Temperature;
        std::wstring UpdateTime;
        std::wstring Weather;
        std::wstring WeatherCode;
        std::wstring AqiPM25;
        std::wstring WindDirection;
        std::wstring WindStrength;

        std::wstring ToString(bool brief) const;
        std::wstring GetTemperature() const;
    };

    struct SWeatherAlert
    {
        std::wstring Type;
        std::wstring Level;
        std::wstring BriefMessage;
        std::wstring DetailedMessage;
        std::wstring PublishTime;

        std::wstring ToString(bool brief) const;
    };

    using WeatherAlertList = std::vector<SWeatherAlert>;

    struct SWeatherInfo
    {
        std::wstring TemperatureDay;
        std::wstring TemperatureNight;
        std::wstring WeatherDay;
        std::wstring WeatherNight;
        std::wstring WeatherCodeDay;
        std::wstring WeatherCodeNight;

        std::wstring GetTemperature() const;
        std::wstring ToString() const;
    };

    bool QueryCity(const std::wstring &query, CityInfoList &info) override;

    std::wstring GetWeatherInfoSummary() override;
    std::wstring GetTemprature(EWeatherInfoType type) override;
    std::wstring GetWeatherText(EWeatherInfoType type) override;
    std::wstring GetWeatherCode(EWeatherInfoType type) override;

    bool UpdateWeather() override;

    bool QueryRealtimeWeather(const std::wstring &query);
    bool QueryForecastWeather(const std::wstring &query);
    bool QueryWeatherAlerts(const std::wstring &query);

protected:
    SRealTimeWeather _realtimeWeather;
    SWeatherInfo _weatherTD, _weatherTM;
    WeatherAlertList _weatherAlerts;
};
