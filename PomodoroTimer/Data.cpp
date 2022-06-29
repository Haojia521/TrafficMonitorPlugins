#include "pch.h"
#include "Data.h"

#include <ctime>
#include <thread>

namespace dm
{
    struct SStateData
    {
        std::time_t m_last_update_timestamp{ 0 };
        std::time_t m_running_time{ 0 };

        int completed_loops{ 0 };
    };

    SStateData state_data;

    bool signal_terminate_update_thread{ false };

    void update_trigger()
    {
        auto &data_madager = CDataManager::Instance();

        while (!signal_terminate_update_thread)
        {
            data_madager.Update();

            // 对齐时间，使下次更新时所使用的时间是10的倍数
            auto sleep_time = 10 - state_data.m_running_time % 10;
            std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
        }
    }
}

CDataManager CDataManager::m_instance;

CDataManager::CDataManager() :
    m_program_state(EProgramState::PS_STOPPED),
    m_pt_state(EPomodoroTimerState::PTS_IN_WORK)
{
    // initialize dpi
    HDC hDC = ::GetDC(HWND_DESKTOP);
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

int CDataManager::DPI(int pixel)
{
    return m_dpi * pixel / 96;
}

float CDataManager::DPIF(float pixel)
{
    return m_dpi * pixel / 96;
}

int CDataManager::RDPI(int pixel)
{
    return pixel * 96 / m_dpi;
}

SConfig& CDataManager::GetConfig()
{
    return m_config;
}

void CDataManager::StartPomodoroTimer()
{
    m_program_state = EProgramState::PS_RUNNING;

    dm::state_data.m_last_update_timestamp = std::time(nullptr);
    
    std::thread update_thread(dm::update_trigger);
    update_thread.detach();
}

void CDataManager::PausePomodoroTimer()
{
    m_program_state = EProgramState::PS_PAUSED;

    dm::signal_terminate_update_thread = true;
}

void CDataManager::StopPomodoroTimer()
{
    m_program_state = EProgramState::PS_STOPPED;
    m_pt_state = EPomodoroTimerState::PTS_IN_WORK;

    dm::state_data.m_running_time = 0;
    dm::state_data.completed_loops = 0;

    dm::signal_terminate_update_thread = true;
}

void CDataManager::SkipCurrentPomodoroTimerState()
{
    if (m_program_state != EProgramState::PS_RUNNING)
        return;

    auto t = std::time(nullptr);
    dm::state_data.m_last_update_timestamp = t;
    dm::state_data.m_running_time = 0;

    if (m_pt_state == EPomodoroTimerState::PTS_IN_WORK)
    {
        m_pt_state = EPomodoroTimerState::PTS_SHORT_BREAK;
    }
    else if (m_pt_state == EPomodoroTimerState::PTS_SHORT_BREAK)
    {
        m_pt_state = EPomodoroTimerState::PTS_IN_WORK;

        dm::state_data.completed_loops += 1;
        if (dm::state_data.completed_loops >= m_config.max_loops)
            StopPomodoroTimer();
    }
}

EProgramState CDataManager::GetProgramState() const
{
    return m_program_state;
}

EPomodoroTimerState CDataManager::GetPomodoroTimerState() const
{
    return m_pt_state;
}

int CDataManager::GetRemaningTime() const
{
    if (m_program_state != EProgramState::PS_RUNNING)
        return 0;

    if (m_pt_state == EPomodoroTimerState::PTS_IN_WORK)
        return m_config.working_time_span - dm::state_data.m_running_time;
    else if (m_pt_state == EPomodoroTimerState::PTS_SHORT_BREAK)
        return m_config.break_time_span - dm::state_data.m_running_time;
    else
        return 0;
}

void CDataManager::Update()
{
    auto t = std::time(nullptr);

    auto past_time = t - dm::state_data.m_last_update_timestamp;
    if (past_time > 60)
    {
        StopPomodoroTimer();
        return;
    }

    dm::state_data.m_running_time += past_time;

    if (m_pt_state == EPomodoroTimerState::PTS_IN_WORK)
    {
        if (dm::state_data.m_running_time >= m_config.working_time_span)
        {
            dm::state_data.m_running_time = 0;
            m_pt_state = EPomodoroTimerState::PTS_SHORT_BREAK;

            // todo: play sound and or meaasge to inform user
        }
    }
    else if (m_pt_state == EPomodoroTimerState::PTS_SHORT_BREAK)
    {
        if (dm::state_data.m_running_time >= m_config.break_time_span)
        {
            dm::state_data.m_running_time = 0;
            m_pt_state = EPomodoroTimerState::PTS_IN_WORK;

            // todo: play sound and or meaasge to inform user

            dm::state_data.completed_loops += 1;
            if (dm::state_data.completed_loops >= m_config.max_loops)
            {
                StopPomodoroTimer();
                return;
            }
        }
    }

    dm::state_data.m_last_update_timestamp = t;
}
