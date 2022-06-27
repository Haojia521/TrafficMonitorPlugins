#include "pch.h"
#include "PomodoroTimer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPomodoroTimer CPomodoroTimer::m_instance;

CPomodoroTimer::CPomodoroTimer()
{}

CPomodoroTimer& CPomodoroTimer::Instance()
{
    return m_instance;
}

IPluginItem* CPomodoroTimer::GetItem(int index)
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

const wchar_t* CPomodoroTimer::GetTooltipInfo()
{
    // todo
    return L"";
}

void CPomodoroTimer::DataRequired()
{
    // todo
}

ITMPlugin::OptionReturn CPomodoroTimer::ShowOptionsDialog(void* hParent)
{
    // todo
    return ITMPlugin::OR_OPTION_NOT_PROVIDED;
}

const wchar_t* CPomodoroTimer::GetInfo(PluginInfoIndex index)
{
    // todo: complete the string resources
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return L"PomodoroTimer";
    case TMI_DESCRIPTION:
        return L"A pomodoro timer plugin of TrafficMonitor";
    case TMI_AUTHOR:
        return L"Haojia521";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Haojia 2022";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/Haojia521/TrafficMonitorPlugins";
        break;
    case TMI_VERSION:
        return L"0.1";
    default:
        break;
    }
    return L"";
}

void CPomodoroTimer::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    // todo: load config info
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        //从配置文件读取配置
        break;
    default:
        break;
    }
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CPomodoroTimer::Instance();
}
