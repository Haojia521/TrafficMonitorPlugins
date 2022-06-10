#pragma once

#include <vector>
#include <string>





class WeatherAPI
{
public:
    virtual bool QueryCityInfo(const std::wstring &query, CityInfoList &city_list) = 0;

    virtual bool QueryRealTimeWeather(const std::wstring &query, SRealTimeWeather &weather) = 0;
    virtual bool QueryWeatherAlerts(const std::wstring &query, WeatherAlertList &alerts) = 0;
    virtual bool QueryForecastWeather(const std::wstring &query, SWeatherInfo &weather_td, SWeatherInfo &weather_tm) = 0;

    virtual std::wstring GetLastError() = 0;
};

class ApiWeatherComCnSpider : public WeatherAPI
{
public:
    bool QueryCityInfo(const std::wstring &query, CityInfoList &city_list) override;

    bool QueryRealTimeWeather(const std::wstring &query, SRealTimeWeather &weather) override;
    bool QueryWeatherAlerts(const std::wstring &query, WeatherAlertList &alerts) override;
    bool QueryForecastWeather(const std::wstring &query, SWeatherInfo &weather_td, SWeatherInfo &weather_tm) override;

    std::wstring GetLastError() override;
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

    std::wstring GetLastError() override;
private:
    std::wstring _key;
    std::wstring _lastError;
};
