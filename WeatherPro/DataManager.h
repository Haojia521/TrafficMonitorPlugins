#pragma once
#include <map>
#include <functional>
#include <memory>

#include "DataApiWeatherComCnSpider.h"
#include "DataApiHefengWeather.h"

enum class UpdateFrequency
{
    UF_3T1H,
    UF_2T1H,
    UF_1T1H,
    UF_1T2H,
    UF_1T3H,
};

enum class DataApiType
{
    API_WeatherComCnSpider,
    API_HefengWeather,
};

enum class IconType
{
    IT_WCC_BULE,
    IT_WCC_WHITE,
    IT_HFW_FILL,
    IT_HFW_HOLLOW,
};

struct SConfiguration
{
    SConfiguration();

    DataApiType m_api_type;
    EWeatherInfoType m_wit;
    UpdateFrequency m_update_frequency;
    IconType m_icon_type;
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
    std::wstring GetTooptipInfo() const;

    SConfiguration& GetConfig();
    const SConfiguration& GetConfig() const;

    void LoadConfigs(const std::wstring &cfg_dir);
    void SaveConfigs() const;

    HICON GetIcon();

    void RefreshWeatherInfoCache();
    
    DataApiPtr GetCurrentApi() const;

    std::shared_ptr<DataApiWeatherComCnSpider> m_api_wccs;
    std::shared_ptr<DataApiHefengWeather> m_api_hfw;
private:
    void _updateWeather(WeatherInfoUpdatedCallback callback = nullptr);
    HICON _getIcon();

    std::wstring _getWeatherTemperature() const;
    std::wstring _getTooptipInfo() const;

    static CDataManager m_instance;
    int m_dpi;
    mutable std::map<UINT, CString> m_string_resources;

    SConfiguration m_config;
    std::wstring m_config_file_path;

    SCityInfo m_currentCityInfo;

    struct SWeatherInfoCache
    {
        std::wstring WeatherTemperature;
        std::wstring TooltipInfo;

        IconType CurrentIconType{ IconType::IT_WCC_BULE };
        HICON Icon{ nullptr };
    };

    SWeatherInfoCache m_weather_info_cache;
};
