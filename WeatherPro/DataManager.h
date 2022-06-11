#pragma once
#include <map>
#include <functional>
#include <memory>

#include "DataApiWeatherComCnSpider.h"
#include "DataApiHefengWeather.h"


enum class DataApiType
{
    API_WeatherComCnSpider,
    API_HefengWeather,
};

struct SConfiguration
{
    SConfiguration();

    DataApiType m_api_type;
    EWeatherInfoType m_wit;
    bool m_show_weather_icon;
    bool m_show_weather_in_tooltips;
    bool m_show_brief_rt_weather_info;
    bool m_show_weather_alerts;
    bool m_show_brief_weather_alert_info;
    bool m_show_error_info;
};

using WeatherInfoUpdatedCallback = std::function<void(const std::wstring & info)>;

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();
public:
    static const CDataManager& Instance();
    static CDataManager& InstanceRef();

    const CString& StringRes(UINT id) const;

    int DPI(int pixel) const;
    float DPIF(float pixel) const;
    int RDPI(int pixel) const;

    void SetCurrentCityInfo(SCityInfo info);
    const SCityInfo& GetCurrentCityInfo() const;

    void UpdateWeather(WeatherInfoUpdatedCallback callback = nullptr);

    std::wstring GetWeatherTemperature() const;
    //std::wstring GetRealTimeWeatherInfo() const;
    //std::wstring GetWeatherAlertsInfo() const;
    //std::wstring GetWeatherInfo() const;

    std::wstring GetTooptipInfo() const;

    SConfiguration& GetConfig();
    const SConfiguration& GetConfig() const;

    void LoadConfigs(const std::wstring &cfg_dir);
    void SaveConfigs() const;

    HICON GetIcon() const;

    void RefreshWeatherInfoCache();
    
    DataApiPtr GetCurrentApi() const;
private:
    void _updateWeather(WeatherInfoUpdatedCallback callback = nullptr);
    HICON _getIcon();
    HICON _getIconByID(UINT id);
    HICON _getIconByCode(const std::wstring& w_code);
    UINT _getIconIdBlue(const std::wstring &code) const;


    std::wstring _getWeatherTemperature() const;
    //std::wstring _getRealTimeWeatherInfo(bool brief) const;
    //std::wstring _getWeatherAlertsInfo(bool brief) const;
    //std::wstring _getWeatherInfo() const;
    std::wstring _getTooptipInfo() const;

    static CDataManager m_instance;
    int m_dpi;
    mutable std::map<UINT, CString> m_string_resources;

    SConfiguration m_config;
    std::wstring m_config_file_path;

    SCityInfo m_currentCityInfo;

    std::map<UINT, HICON> m_icons;

    struct SWeatherInfoCache
    {
        std::wstring WeatherTemperature;
        //std::wstring RealTimeWeatherInfo;
        //std::wstring WeatherAlersInfo;
        //std::wstring WeatherInfo;
        std::wstring TooltipInfo;

        HICON Icon{ nullptr };
    };

    SWeatherInfoCache m_weather_info_cache;

    std::shared_ptr<DataApiWeatherComCnSpider> m_api_wccs;
    std::shared_ptr<DataApiHefengWeather> m_api_hfw;
};
