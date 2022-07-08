#include "pch.h"
#include "DataManager.h"
#include "resource.h"

#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>

std::mutex g_weather_update_nutex;

SConfiguration::SConfiguration() :
    m_api_type(DataApiType::API_HefengWeather),
    m_wit(EWeatherInfoType::WEATHER_REALTIME),
    m_update_frequency(UpdateFrequency::UF_1T2H),
    m_show_weather_icon(true),
    m_show_weather_in_tooltips(true),
    m_show_brief_rt_weather_info(false),
    m_show_weather_alerts(true),
    m_show_brief_weather_alert_info(true),
    m_show_error_info(false)
{}

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
    HDC hDC = ::GetDC(HWND_DESKTOP);
    m_dpi = ::GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);

    m_api_wccs = std::make_shared<DataApiWeatherComCnSpider>();
    m_api_hfw = std::make_shared<DataApiHefengWeather>();
}

CDataManager::~CDataManager()
{
    SaveConfigs();
}

const CDataManager& CDataManager::Instance()
{
    return m_instance;
}

CDataManager& CDataManager::InstanceRef()
{
    return m_instance;
}

const CString& CDataManager::StringRes(UINT id) const
{
    if (m_string_resources.count(id) == 0)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_resources[id].LoadString(id);
    }

    return m_string_resources[id];
}

int CDataManager::DPI(int pixel) const
{
    return m_dpi * pixel / 96;
}

float CDataManager::DPIF(float pixel) const
{
    return m_dpi * pixel / 96;
}

int CDataManager::RDPI(int pixel) const
{
    return pixel * 96 / m_dpi;
}

void CDataManager::SetCurrentCityInfo(SCityInfo info)
{
    m_currentCityInfo = info;
}

const SCityInfo& CDataManager::GetCurrentCityInfo() const
{
    return m_currentCityInfo;
}

DataApiPtr CDataManager::GetCurrentApi() const
{
    if (m_config.m_api_type == DataApiType::API_WeatherComCnSpider)
        return m_api_wccs;
    else if (m_config.m_api_type == DataApiType::API_HefengWeather)
        return m_api_hfw;
    else
        return nullptr;
}

void CDataManager::_updateWeather(WeatherInfoUpdatedCallback callback)
{
    std::lock_guard<std::mutex> guard(g_weather_update_nutex);

    std::shared_ptr<DataAPI> current_api{ GetCurrentApi() };
    if (current_api != nullptr)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (current_api->UpdateWeather())
                break;

            std::this_thread::sleep_for(std::chrono::seconds(3));
        }

        RefreshWeatherInfoCache();

        if (callback != nullptr)
        {
            callback(GetTooptipInfo());
        }
    }
}

void CDataManager::UpdateWeather(WeatherInfoUpdatedCallback callback /* = nullptr */)
{
    std::thread t(&CDataManager::_updateWeather, this, callback);
    t.detach();
}

std::wstring CDataManager::GetWeatherTemperature() const
{
    return m_weather_info_cache.WeatherTemperature;
}

std::wstring CDataManager::GetTooptipInfo() const
{
    return m_weather_info_cache.TooltipInfo;
}

SConfiguration& CDataManager::GetConfig()
{
    return m_config;
}

const SConfiguration& CDataManager::GetConfig() const
{
    return m_config;
}

void CDataManager::LoadConfigs(const std::wstring &cfg_dir)
{
    const auto &ch = cfg_dir.back();
    if (ch == L'\\' || ch == L'/')
        m_config_file_path = cfg_dir + L"WeatherPro.ini";
    else
        m_config_file_path = cfg_dir + L"\\WeatherPro.ini";

    auto cfg_int_val_getter = [this](const wchar_t *section, const wchar_t *key, int default_val) {
        int val = GetPrivateProfileInt(section, key, default_val, m_config_file_path.c_str());
        return val;
    };

    auto cfg_str_val_getter = [this](const wchar_t *section, const wchar_t *key, const wchar_t *default_val) {
        wchar_t buffer[64]{ 0 };
        GetPrivateProfileString(section, key, default_val, buffer, 64, m_config_file_path.c_str());
        return std::wstring{ buffer };
    };

    auto cfg_bool_val_getter = [cfg_int_val_getter](const wchar_t *section, const wchar_t *key, int default_val) {
        return cfg_int_val_getter(section, key, default_val) != 0;
    };

    m_currentCityInfo.CityNO = cfg_str_val_getter(L"config", L"city_no", L"101010100");
    m_currentCityInfo.CityName = cfg_str_val_getter(L"config", L"city_name", L"北京");

    m_config.m_api_type = static_cast<DataApiType>(cfg_int_val_getter(L"config", L"api_type", 1));
    m_config.m_wit = static_cast<EWeatherInfoType>(cfg_int_val_getter(L"config", L"wit", 0));
    m_config.m_update_frequency = static_cast<UpdateFrequency>(cfg_int_val_getter(L"config", L"update_freq", 1));
    m_config.m_show_weather_icon = cfg_bool_val_getter(L"config", L"show_weather_icon", 1);
    m_config.m_show_weather_in_tooltips = cfg_bool_val_getter(L"config", L"show_weather_in_tooltips", 1);
    m_config.m_show_brief_rt_weather_info = cfg_bool_val_getter(L"config", L"show_brief_rt_weather_info", 0);
    m_config.m_show_weather_alerts = cfg_bool_val_getter(L"config", L"show_weather_alerts", 1);
    m_config.m_show_brief_weather_alert_info = cfg_bool_val_getter(L"config", L"show_brief_weather_alert_info", 1);
    m_config.m_show_error_info = cfg_bool_val_getter(L"config", L"show_error_info", 0);

    auto &hf_cfg = m_api_hfw->config;
    hf_cfg.AppKey = cfg_str_val_getter(L"hfw", L"AppKey", L"");
    hf_cfg.ShowRealtimeTemperatureFeelsLike = cfg_bool_val_getter(L"hfw", L"show_rt_temp_feels_like", 0);
    hf_cfg.ShowRealtimeWind = cfg_bool_val_getter(L"hfw", L"show_rt_wind", 1);
    hf_cfg.ShowRealtimeWindScale = cfg_bool_val_getter(L"hfw", L"show_rt_wind_scale", 1);
    hf_cfg.ShowRealtimeHumidity = cfg_bool_val_getter(L"hfw", L"show_rt_humidity", 1);
    hf_cfg.ShowForecastUVIdex = cfg_bool_val_getter(L"hfw", L"show_fc_uv_index", 0);
    hf_cfg.showForecastHumidity = cfg_bool_val_getter(L"hfw", L"show_fc_humidity", 0);
    hf_cfg.ShowAirQuality = cfg_bool_val_getter(L"hfw", L"show_rt_air_quality", 1);
    hf_cfg.ShowAirQualityAQI = cfg_bool_val_getter(L"hfw", L"show_rt_air_quality_aqi", 1);
    hf_cfg.ShowAirQualityPM2p5 = cfg_bool_val_getter(L"hfw", L"show_rt_air_quality_pm2p5", 1);
    hf_cfg.ShowAirQualityPM10 = cfg_bool_val_getter(L"hfw", L"show_rt_air_quality_pm10", 0);
    hf_cfg.ShowWeatherAlert = cfg_bool_val_getter(L"hfw", L"show_weather_alert", 1);
}

void CDataManager::SaveConfigs() const
{
    auto cfg_int_val_writter = [this](const wchar_t *section, const wchar_t *key, int value) {
        wchar_t buffer[64]{ 0 };

        swprintf_s(buffer, L"%d", value);
        WritePrivateProfileString(section, key, buffer, m_config_file_path.c_str());
    };

    auto cfg_str_val_writter = [this](const wchar_t *section, const wchar_t *key, const wchar_t *value) {
        WritePrivateProfileString(section, key, value, m_config_file_path.c_str());
    };

    auto cfg_bool_val_writter = [cfg_int_val_writter](const wchar_t *section, const wchar_t *key, bool value) {
        cfg_int_val_writter(section, key, value ? 1 : 0);
    };

    cfg_str_val_writter(L"config", L"city_no", m_currentCityInfo.CityNO.c_str());
    cfg_str_val_writter(L"config", L"city_name", m_currentCityInfo.CityName.c_str());

    cfg_int_val_writter(L"config", L"api_type", static_cast<int>(m_config.m_api_type));
    cfg_int_val_writter(L"config", L"wit", static_cast<int>(m_config.m_wit));
    cfg_int_val_writter(L"config", L"update_freq", static_cast<int>(m_config.m_update_frequency));
    cfg_bool_val_writter(L"config", L"show_weather_icon", m_config.m_show_weather_icon);
    cfg_bool_val_writter(L"config", L"show_weather_icon", m_config.m_show_weather_icon);
    cfg_bool_val_writter(L"config", L"show_weather_in_tooltips", m_config.m_show_weather_in_tooltips);
    cfg_bool_val_writter(L"config", L"show_brief_rt_weather_info", m_config.m_show_brief_rt_weather_info);
    cfg_bool_val_writter(L"config", L"show_weather_alerts", m_config.m_show_weather_alerts);
    cfg_bool_val_writter(L"config", L"show_brief_weather_alert_info", m_config.m_show_brief_weather_alert_info);
    cfg_bool_val_writter(L"config", L"show_error_info", m_config.m_show_error_info);

    auto &hf_cfg = m_api_hfw->config;
    cfg_str_val_writter(L"hfw", L"AppKey", hf_cfg.AppKey.c_str());
    cfg_bool_val_writter(L"hfw", L"show_rt_temp_feels_like", hf_cfg.ShowRealtimeTemperatureFeelsLike);
    cfg_bool_val_writter(L"hfw", L"show_rt_wind", hf_cfg.ShowRealtimeWind);
    cfg_bool_val_writter(L"hfw", L"show_rt_wind_scale", hf_cfg.ShowRealtimeWindScale);
    cfg_bool_val_writter(L"hfw", L"show_rt_humidity", hf_cfg.ShowRealtimeHumidity);
    cfg_bool_val_writter(L"hfw", L"show_fc_uv_index", hf_cfg.ShowForecastUVIdex);
    cfg_bool_val_writter(L"hfw", L"show_fc_humidity", hf_cfg.showForecastHumidity);
    cfg_bool_val_writter(L"hfw", L"show_rt_air_quality", hf_cfg.ShowAirQuality);
    cfg_bool_val_writter(L"hfw", L"show_rt_air_quality_aqi", hf_cfg.ShowAirQualityAQI);
    cfg_bool_val_writter(L"hfw", L"show_rt_air_quality_pm2p5", hf_cfg.ShowAirQualityPM2p5);
    cfg_bool_val_writter(L"hfw", L"show_rt_air_quality_pm10", hf_cfg.ShowAirQualityPM10);
    cfg_bool_val_writter(L"hfw", L"show_weather_alert", hf_cfg.ShowWeatherAlert);
}

HICON CDataManager::_getIconByID(UINT id)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, DPI(16), DPI(16), 0);

    return hIcon;
}

UINT CDataManager::_getIconIdBlue(const std::wstring& code) const
{
    static const std::map<std::wstring, UINT> dmap{
        {L"d00", IDI_ICON_B_D00},
        {L"d01", IDI_ICON_B_D01},
        {L"d03", IDI_ICON_B_D03},
        {L"d13", IDI_ICON_B_D13},
        {L"n00", IDI_ICON_B_N00},
        {L"n01", IDI_ICON_B_N01},
        {L"n03", IDI_ICON_B_N03},
        {L"n13", IDI_ICON_B_N13},
        {L"02", IDI_ICON_B_A02},
        {L"04", IDI_ICON_B_A04},
        {L"05", IDI_ICON_B_A05},
        {L"06", IDI_ICON_B_A06},
        {L"07", IDI_ICON_B_A07},
        {L"08", IDI_ICON_B_A08},
        {L"09", IDI_ICON_B_A09},
        {L"10", IDI_ICON_B_A10},
        {L"11", IDI_ICON_B_A11},
        {L"12", IDI_ICON_B_A12},
        {L"14", IDI_ICON_B_A14},
        {L"15", IDI_ICON_B_A15},
        {L"16", IDI_ICON_B_A16},
        {L"17", IDI_ICON_B_A17},
        {L"18", IDI_ICON_B_A18},
        {L"19", IDI_ICON_B_A19},
        {L"20", IDI_ICON_B_A20},
        {L"29", IDI_ICON_B_A29},
        {L"30", IDI_ICON_B_A30},
        {L"31", IDI_ICON_B_A31},
        {L"53", IDI_ICON_B_A53},
        {L"54", IDI_ICON_B_A54},
        {L"55", IDI_ICON_B_A55},
        {L"56", IDI_ICON_B_A56},
        {L"99", IDI_ICON_B_A99},
        {L"21", IDI_ICON_B_A08},
        {L"22", IDI_ICON_B_A09},
        {L"23", IDI_ICON_B_A10},
        {L"24", IDI_ICON_B_A11},
        {L"25", IDI_ICON_B_A12},
        {L"26", IDI_ICON_B_A15},
        {L"27", IDI_ICON_B_A16},
        {L"28", IDI_ICON_B_A17},
        {L"32", IDI_ICON_B_A18},
        {L"49", IDI_ICON_B_A18},
        {L"57", IDI_ICON_B_A18},
        {L"58", IDI_ICON_B_A18},
        {L"97", IDI_ICON_B_A08},
        {L"98", IDI_ICON_B_A15},
        {L"301", IDI_ICON_B_A08},
        {L"302", IDI_ICON_B_A15},
    };

    auto itr = dmap.find(code);
    if (itr != dmap.end())
        return itr->second;
    else if (code.size() > 1)
    {
        auto itr2 = dmap.find(code.substr(1));
        if (itr2 != dmap.end())
            return itr2->second;
    }

    return IDI_ICON_B_A99;
}

HICON CDataManager::GetIcon() const
{
    return m_weather_info_cache.Icon;
}

HICON CDataManager::_getIcon()
{
    auto api = GetCurrentApi();
    if (api != nullptr)
        return _getIconByCode(api->GetWeatherCode(m_config.m_wit));
    else
        return nullptr;
}

HICON CDataManager::_getIconByCode(const std::wstring& w_code)
{
    if (w_code.empty()) return nullptr;

    auto icon_id = _getIconIdBlue(w_code);

    if (m_icons.find(icon_id) == m_icons.end())
    {
        m_icons[icon_id] = _getIconByID(icon_id);
    }

    return m_icons[icon_id];
}

std::wstring CDataManager::_getWeatherTemperature() const
{
    auto api = GetCurrentApi();
    if (api != nullptr)
    {
        if (m_config.m_show_weather_icon)
            return api->GetTemprature(m_config.m_wit);
        else
            return api->GetWeatherText(m_config.m_wit) + L" " + api->GetTemprature(m_config.m_wit);
    }
    else
        return L"";
}

std::wstring CDataManager::_getTooptipInfo() const
{
    auto api = GetCurrentApi();
    if (api != nullptr)
    {
        std::wstringstream wss;

        wss << m_currentCityInfo.CityName << L" " << api->GetWeatherInfoSummary();

        auto lastErr = api->GetLastError();
        if (m_config.m_show_error_info && !lastErr.empty())
        {
            wss << std::endl;

            wss << L"=====WeatherPro-Errors=====" << std::endl
                << lastErr << std::endl
                << L"===========================";
        }

        return wss.str();
    }

    return L"";
}

void CDataManager::RefreshWeatherInfoCache()
{
    m_weather_info_cache.WeatherTemperature = _getWeatherTemperature();
    m_weather_info_cache.TooltipInfo = _getTooptipInfo();
    m_weather_info_cache.Icon = _getIcon();
}
