#pragma once

#include <vector>
#include <string>

struct SCityInfo
{
    std::wstring CityNO;
    std::wstring CityName;
    std::wstring CityAdministrativeOwnership;
};

using CityInfoList = std::vector<SCityInfo>;

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

class WeatherAPI
{
public:
    virtual bool QueryCityInfo(const std::wstring &query, CityInfoList &city_list) = 0;

    virtual bool QueryRealTimeWeather(const std::wstring &query, SRealTimeWeather &weather) = 0;
    virtual bool QueryWeatherAlerts(const std::wstring &query, WeatherAlertList &alerts) = 0;
    virtual bool QueryForecastWeather(const std::wstring &query, SWeatherInfo &weather_td, SWeatherInfo &weather_tm) = 0;

    virtual const std::wstring& GetLastError() = 0;
};

class ApiWeatherComCnSpider : public WeatherAPI
{
public:
    bool QueryCityInfo(const std::wstring &query, CityInfoList &city_list) override;

    bool QueryRealTimeWeather(const std::wstring &query, SRealTimeWeather &weather) override;
    bool QueryWeatherAlerts(const std::wstring &query, WeatherAlertList &alerts) override;
    bool QueryForecastWeather(const std::wstring &query, SWeatherInfo &weather_td, SWeatherInfo &weather_tm) override;

    const std::wstring& GetLastError() override;
};

class ApiHefengWeather : public WeatherAPI
{
public:
    ApiHefengWeather(const std::wstring &key);

    const std::wstring &GetKey() const;
    void SetKey(const std::wstring &key);

    bool QueryCityInfo(const std::wstring &query, CityInfoList &city_list) override;

    bool QueryRealTimeWeather(const std::wstring &query, SRealTimeWeather &weather) override;
    bool QueryWeatherAlerts(const std::wstring &query, WeatherAlertList &alerts) override;
    bool QueryForecastWeather(const std::wstring &query, SWeatherInfo &weather_td, SWeatherInfo &weather_tm) override;

    const std::wstring& GetLastError() override;
private:
    std::wstring _key;
    std::wstring _lastError;
};
