#include "pch.h"
#include "Resource.h"
#include "PomodoroTimer.h"
#include "Data.h"
#include "OptionsDlg.h"

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
    return L"";
}

void CPomodoroTimer::DataRequired()
{
}

ITMPlugin::OptionReturn CPomodoroTimer::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWnd* pParent = CWnd::FromHandle((HWND)hParent);

    COptionsDlg dlg(pParent);
    if (dlg.DoModal() == IDOK)
        return ITMPlugin::OR_OPTION_CHANGED;
    
    return ITMPlugin::OR_OPTION_UNCHANGED;
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
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        // 从配置文件读取配置
        CDataManager::Instance().LoadConfig(data);
        break;
    default:
        break;
    }
}

void CPomodoroTimer::ShowContextMenu(CWnd *wnd)
{
    static CMenu menu;
    if (menu.m_hMenu == nullptr)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        menu.LoadMenu(IDR_MENU_MAIN);
    }

    CMenu *context_menu = menu.GetSubMenu(0);
    if (context_menu == nullptr) return;
    
    auto &data_manager = CDataManager::Instance();
    auto prog_state = data_manager.GetProgramState();

    // initialize menu item
    if (prog_state == EProgramState::PS_RUNNING)
        context_menu->EnableMenuItem(ID_FUNC_START, MF_DISABLED);
    else
        context_menu->EnableMenuItem(ID_FUNC_START, MF_ENABLED);

    if (data_manager.GetProgramState() == EProgramState::PS_RUNNING)
        context_menu->EnableMenuItem(ID_FUNC_PAUSE, MF_ENABLED);
    else
        context_menu->EnableMenuItem(ID_FUNC_PAUSE, MF_DISABLED);

    if (data_manager.GetProgramState() == EProgramState::PS_STOPPED)
        context_menu->EnableMenuItem(ID_FUNC_SKIP, MF_DISABLED);
    else
        context_menu->EnableMenuItem(ID_FUNC_SKIP, MF_ENABLED);

    if (data_manager.GetProgramState() == EProgramState::PS_STOPPED)
        context_menu->EnableMenuItem(ID_FUNC_STOP, MF_DISABLED);
    else
        context_menu->EnableMenuItem(ID_FUNC_STOP, MF_ENABLED);

    // show menu
    CPoint point;
    GetCursorPos(&point);
    auto id = context_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, wnd);

    if (id == ID_FUNC_START)
        data_manager.StartPomodoroTimer();
    else if (id == ID_FUNC_PAUSE)
        data_manager.PausePomodoroTimer();
    else if (id == ID_FUNC_SKIP)
        data_manager.SkipCurrentPomodoroTimerState();
    else if (id == ID_FUNC_STOP)
        data_manager.StopPomodoroTimer();
    else if (id == ID_FUNC_OPTIONS)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        COptionsDlg dlg(wnd);
        dlg.DoModal();
    }
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CPomodoroTimer::Instance();
}
