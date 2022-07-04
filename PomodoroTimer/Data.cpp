#include "pch.h"
#include "Data.h"

#include <ctime>
#include <thread>
#include <memory>

namespace dm
{
    struct SStateData
    {
        std::time_t m_last_update_timestamp{ 0 };
        std::time_t m_running_time{ 0 };

        int completed_loops{ 0 };
    };

    SStateData state_data;

    void update_func(std::shared_ptr<bool> signal_stop)
    {
        auto &data_madager = CDataManager::Instance();

        while (signal_stop != nullptr && !(*signal_stop))
        {
            data_madager.Update();

            // 对齐时间，使下次更新时所使用的时间是10的倍数
            auto sleep_time = 10 - state_data.m_running_time % 10;
            std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
        }
    }

    class UpdateThreadHelper
    {
    public:
        void start()
        {
            // if current thread is still running, do nothing
            if (_signal_stop != nullptr && !(*_signal_stop))
                return;

            _signal_stop = std::make_shared<bool>(false);

            std::thread update_thread(update_func, _signal_stop);
            update_thread.detach();
        }

        void stop()
        {
            *_signal_stop = true;
        }
    private:
        std::shared_ptr<bool> _signal_stop{ nullptr };
    };

    UpdateThreadHelper thread_helper;

    int get_sound_resource_id(int id)
    {
        switch (id)
        {
        default:
        case 0:
            return IDR_WAVE1;

        case 1:
            return IDR_WAVE2;

        case 2:
            return IDR_WAVE3;
        }
    }

    void play_sound(int id)
    {
        auto sound_res_id = get_sound_resource_id(id);

        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        PlaySound(MAKEINTRESOURCE(sound_res_id), AfxGetResourceHandle(), SND_ASYNC | SND_RESOURCE);
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

const CString& CDataManager::StringRes(UINT id)
{
    if (m_string_res_map.count(id) == 0)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_res_map[id].LoadString(id);
    }

    return m_string_res_map[id];
}

SConfig& CDataManager::GetConfig()
{
    return m_config;
}

void CDataManager::LoadConfig(const std::wstring &cfg_dir)
{
    const auto &ch = cfg_dir.back();
    if (ch == L'\\' || ch == L'/')
        m_config_file_path = cfg_dir + L"PomodoroTimer.ini";
    else
        m_config_file_path = cfg_dir + L"\\PomodoroTimer.ini";

    auto cfg_int_val_getter = [this](const wchar_t *section, const wchar_t *key, int default_val) {
        int val = GetPrivateProfileInt(section, key, default_val, m_config_file_path.c_str());
        return val;
    };

    auto cfg_bool_val_getter = [cfg_int_val_getter](const wchar_t *section, const wchar_t *key, int default_val) {
        return cfg_int_val_getter(section, key, default_val) != 0;
    };

    auto timespan_work = cfg_int_val_getter(L"config", L"timespan_work", 1500);
    if (timespan_work < 60) timespan_work = 60;
    m_config.working_time_span = timespan_work / 60 * 60;

    auto timespan_break = cfg_int_val_getter(L"config", L"timespan_break", 300);
    if (timespan_break < 60) timespan_break = 60;
    m_config.break_time_span = timespan_break / 60 * 60;

    m_config.auto_start = cfg_bool_val_getter(L"config", L"auto_start", 0);

    m_config.auto_loop = cfg_bool_val_getter(L"config", L"auto_loop", 0);

    auto num_loops = cfg_int_val_getter(L"config", L"num_loops", 4);
    if (num_loops < 0) num_loops = 0;
    if (num_loops > 100) num_loops = 100;
    m_config.max_loops = num_loops;

    m_config.play_sound = cfg_bool_val_getter(L"config", L"play_sound", 1);

    auto sound_id = cfg_int_val_getter(L"config", L"sound_id", 0);
    if (sound_id < 0 || sound_id >= 3) sound_id = 0;
    m_config.sound_id = sound_id;
}

void CDataManager::SaveConfig() const
{
    auto cfg_int_val_writter = [this](const wchar_t *section, const wchar_t *key, int value) {
        wchar_t buffer[64]{ 0 };

        swprintf_s(buffer, L"%d", value);
        WritePrivateProfileString(section, key, buffer, m_config_file_path.c_str());
    };

    auto cfg_bool_val_writter = [cfg_int_val_writter](const wchar_t *section, const wchar_t *key, bool value) {
        cfg_int_val_writter(section, key, value ? 1 : 0);
    };

    cfg_int_val_writter(L"config", L"timespan_work", m_config.working_time_span);
    cfg_int_val_writter(L"config", L"timespan_break", m_config.break_time_span);
    cfg_bool_val_writter(L"config", L"auto_start", m_config.auto_start);
    cfg_bool_val_writter(L"config", L"auto_loop", m_config.auto_loop);
    cfg_int_val_writter(L"config", L"num_loops", m_config.max_loops);
    cfg_bool_val_writter(L"config", L"play_sound", m_config.play_sound);
    cfg_int_val_writter(L"config", L"sound_id", m_config.sound_id);
}

void CDataManager::StartPomodoroTimer()
{
    m_program_state = EProgramState::PS_RUNNING;

    dm::state_data.m_last_update_timestamp = std::time(nullptr);

    dm::thread_helper.start();
}

void CDataManager::PausePomodoroTimer()
{
    m_program_state = EProgramState::PS_PAUSED;

    dm::thread_helper.stop();
}

void CDataManager::StopPomodoroTimer()
{
    m_program_state = EProgramState::PS_STOPPED;
    m_pt_state = EPomodoroTimerState::PTS_IN_WORK;

    dm::state_data.m_running_time = 0;
    dm::state_data.completed_loops = 0;

    dm::thread_helper.stop();
}

void CDataManager::SkipCurrentPomodoroTimerState()
{
    if (m_program_state == EProgramState::PS_STOPPED)
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

        if (!NextLoop()) return;
    }

    if (m_program_state == EProgramState::PS_PAUSED)
        StartPomodoroTimer();
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
        return static_cast<int>(m_config.working_time_span - dm::state_data.m_running_time);
    else if (m_pt_state == EPomodoroTimerState::PTS_SHORT_BREAK)
        return static_cast<int>(m_config.break_time_span - dm::state_data.m_running_time);
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

            if (m_config.play_sound) PlaySoundById(m_config.sound_id);
        }
    }
    else if (m_pt_state == EPomodoroTimerState::PTS_SHORT_BREAK)
    {
        if (dm::state_data.m_running_time >= m_config.break_time_span)
        {
            dm::state_data.m_running_time = 0;
            m_pt_state = EPomodoroTimerState::PTS_IN_WORK;

            if (m_config.play_sound) PlaySoundById(m_config.sound_id);

            if (!NextLoop()) return;
        }
    }

    dm::state_data.m_last_update_timestamp = t;
}

void CDataManager::PlaySoundById(int id) const
{
    dm::play_sound(id);
}

// go to next loop.
// return true if the timer will continue running into next loop, otherwise false.
bool CDataManager::NextLoop()
{
    dm::state_data.completed_loops += 1;
    if (!m_config.auto_loop ||
        (m_config.max_loops > 0 && dm::state_data.completed_loops >= m_config.max_loops))
    {
        StopPomodoroTimer();
        return false;
    }

    return true;
}
