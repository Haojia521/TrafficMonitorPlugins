﻿// WeatherPro.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "WeatherPro.h"
#include "DataManager.h"

#include "OptionsDlg.h"

#include <cstdlib>

namespace helper
{
    std::time_t gen_time_span(std::time_t t)
    {
        std::srand(static_cast<unsigned int>(t));
        auto rand_number = std::rand();

        switch (CDataManager::Instance().GetConfig().m_update_frequency)
        {
        case UpdateFrequency::UF_3T1H:
            return static_cast<std::time_t>(900 + rand_number % 600);

        default:
        case UpdateFrequency::UF_2T1H:
            return static_cast<std::time_t>(1500 + rand_number % 600);

        case UpdateFrequency::UF_1T1H:
            return static_cast<std::time_t>(3300 + rand_number % 600);

        case UpdateFrequency::UF_1T2H:
            return static_cast<std::time_t>(6600 + rand_number % 1200);

        case UpdateFrequency::UF_1T3H:
            return static_cast<std::time_t>(9600 + rand_number % 2400);
        }
    }
}

CWeatherPro CWeatherPro::m_instance;

CWeatherPro::CWeatherPro() :
    m_last_update_timestamp{ 0 },
    m_next_update_time_span{ 300 }
{}

CWeatherPro& CWeatherPro::Instance()
{
    return m_instance;
}

IPluginItem* CWeatherPro::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;

    default:
        break;
    }

    return nullptr;
}

void CWeatherPro::DataRequired()
{
    UpdateWeatherInfo();
}

const wchar_t* CWeatherPro::GetInfo(PluginInfoIndex index)
{
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return L"WeatherPro";
    case TMI_DESCRIPTION:
        return L"用于显示天气的TrafficMonitor插件";
    case TMI_AUTHOR:
        return L"Haojia521";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Haojia 2022";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/Haojia521/TrafficMonitorPlugins";
        break;
    case TMI_VERSION:
        return L"0.6.4";
    default:
        break;
    }
    return L"";
}

const wchar_t* CWeatherPro::GetTooltipInfo()
{
    if (CDataManager::Instance().GetConfig().m_show_weather_in_tooltips)
        return m_tooltips_info.c_str();
    else
        return L"";
}

ITMPlugin::OptionReturn CWeatherPro::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWnd *parent = CWnd::FromHandle((HWND)hParent);
    COptionsDlg dlg(parent);
    dlg.DoModal();

    return ITMPlugin::OR_OPTION_CHANGED;
}

void CWeatherPro::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        CDataManager::InstanceRef().LoadConfigs(data);
        break;

    default:
        break;
    }
}

void CWeatherPro::UpdateTooltip(const std::wstring &info)
{
    m_tooltips_info = info;
}

void CWeatherPro::UpdateWeatherInfo(bool force /* = false */)
{
    static auto cb = [this](const std::wstring &info) {
        this->m_tooltips_info = info;
    };

    auto t = std::time(nullptr);

    if (force)
    {
        m_last_update_timestamp = t;
        CDataManager::InstanceRef().UpdateWeather(cb);
    }
    else
    {
        if (t > m_last_update_timestamp + m_next_update_time_span)
        {
            m_last_update_timestamp = t;
            m_next_update_time_span = helper::gen_time_span(t);

            CDataManager::InstanceRef().UpdateWeather(cb);
        }
    }
}

std::time_t CWeatherPro::GetLastUpdateTimestamp() const
{
    return m_last_update_timestamp;
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CWeatherPro::Instance();
}
