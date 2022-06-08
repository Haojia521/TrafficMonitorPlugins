#include "pch.h"
#include "DataQuerier.h"

#include <ctime>
#include <sstream>


std::wstring SRealTimeWeather::ToString(bool brief) const
{
    std::wstringstream wss;

    wss << Weather << " " << Temperature << L"℃ (" << UpdateTime << L')';
    if (!brief)
    {
        wss << L" PM2.5: " << AqiPM25 << "  " << WindDirection << WindStrength;
    }

    return wss.str();
}

std::wstring SRealTimeWeather::GetTemperature() const
{
    return Temperature + L"℃";
}

std::wstring SWeatherAlert::ToString(bool brief) const
{
    std::wstringstream wss;

    if (brief)
    {
        wss << BriefMessage << L" (" << PublishTime << L')';
    }
    else
    {
        wss << DetailedMessage;
    }

    return wss.str();
}

std::wstring SWeatherInfo::ToString() const
{
    std::wstringstream wss;

    if (WeatherDay == WeatherNight)
        wss << WeatherDay << L' ';
    else
        wss << WeatherDay << L"转" << WeatherNight << L' ';

    wss << TemperatureNight << L"~" << TemperatureDay << L"℃";

    return wss.str();
}

std::wstring SWeatherInfo::GetTemperature() const
{
    std::wstringstream wss;
    wss << TemperatureNight << L"~" << TemperatureDay << L"℃";

    return wss.str();
}
