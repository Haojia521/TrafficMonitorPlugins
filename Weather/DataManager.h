﻿#pragma once
#include <string>
#include <map>
#include "resource.h"
#include "CityCode.h"

#define g_data CDataManager::Instance()

enum WeahterSelected
{
    WEATHER_TODAY,
    WEATHER_TOMMORROW
};

struct SettingData
{
    int m_city_index{};                     //选择的城市在列表中的序号
    WeahterSelected m_weather_selected{};   //要显示的天气
    bool m_show_weather_in_tooltips{};      //是否在鼠标提示中显示
};

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();

    void LoadConfig();
    void SaveConfig() const;
    const CString& StringRes(UINT id);      //根据资源id获取一个字符串资源
    void DPIFromWindow(CWnd* pWnd);
    int DPI(int pixel);
    HICON GetIcon(UINT id);
    CityCodeItem CurCity() const;
    void ResetText();

    struct WeatherInfo
    {
        std::wstring m_type = L"--";
        std::wstring m_high = L"-℃";
        std::wstring m_low = L"-℃";
        std::wstring ToString() const;
    };
    WeatherInfo& GetWeather();

    std::map<WeahterSelected, WeatherInfo> m_weather_info;

    SettingData m_setting_data;

public:
    const std::wstring& GetModulePath() const;

private:
    static CDataManager m_instance;
    std::wstring m_module_path;
    std::map<UINT, CString> m_string_table;
    std::map<UINT, HICON> m_icons;
    int m_dpi{ 96 };
};
