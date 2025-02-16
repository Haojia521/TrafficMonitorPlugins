#pragma once
#include "DataAPI.h"

class DataApiHefengWeather : public DataAPI
{
public:
    struct Config
    {
        std::wstring AppKey;

        bool ShowRealtimeTemperatureFeelsLike{ false };
        bool ShowRealtimeWind{ true };
        bool ShowRealtimeWindScale{ true };
        bool ShowRealtimeHumidity{ true };
        bool ShowForecastUVIdex{ false };
        bool showForecastHumidity{ false };
        bool ShowAirQuality{ true };
        bool ShowAirQualityAQI{ true };
        bool ShowAirQualityPM2p5{ true };
        bool ShowAirQualityPM10{ false };
        bool ShowWeatherAlert{ true };
    };

    struct RealtimeWeather
    {
        std::wstring Temperature;
        std::wstring TemperatureFeelsLike;
        std::wstring UpdateTime;
        std::wstring WeatherText;
        std::wstring WeatherCode;
        std::wstring WindDirection;
        std::wstring WindScale;
        std::wstring WindSpeed;
        std::wstring Humidity;
    };

    struct RealtimeAirQuality
    {
        std::wstring PublishTime;
        std::wstring AQI;
        std::wstring Level;
        std::wstring Category;
        std::wstring PM2p5;
        std::wstring PM10;
    };

    struct ForcastWeather
    {
        std::wstring TemperatureMax;
        std::wstring TemperatureMin;
        std::wstring WeatherDay;
        std::wstring WeatherNight;
        std::wstring WeatherCodeDay;
        std::wstring WeatherCodeNight;
        std::wstring UVIndex;
        std::wstring Humidity;
    };

    struct WeatherAlert
    {
        std::wstring Type;
        std::wstring TypeName;
        std::wstring Level;
        std::wstring Title;
        std::wstring Text;
        std::wstring Severity;
        std::wstring SeverityColor;
        std::wstring PublishTime;
    };

    using WeatherAlertList = std::vector<WeatherAlert>;

    bool QueryCity(const std::wstring &query, CityInfoList &info, WStringList &errors) override;

    std::wstring GetWeatherInfoSummary(WStringList &errors) override;
    std::wstring GetTemprature(EWeatherInfoType type) override;
    std::wstring GetWeatherText(EWeatherInfoType type) override;
    std::wstring GetWeatherCode(EWeatherInfoType type) override;

    bool UpdateWeather(WStringList &errors) override;

    bool QueryRealtimeWeather(const std::wstring &query, WStringList &errors);
    bool QueryForecastWeather(const std::wstring &query, WStringList &errors);
    bool QueryRealtimeAirQuality(const std::wstring &query, WStringList &errors);
    bool QueryWeatherAlerts(const std::wstring &query, WStringList &errors);

    Config config;
protected:
    RealtimeWeather _realtimeWeather;
    ForcastWeather _forcastWeatherTD, _forcastWeatherTM, _forcastWeatherDATM;
    RealtimeAirQuality _realtimeAirQuality;
    WeatherAlertList _weatherAlerts;

    bool _airQualityDataOutdated{ true };
    bool _alertsDataOutdated{ true };
};
