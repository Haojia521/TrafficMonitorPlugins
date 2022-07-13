﻿#pragma once

#include "Resource.h"
#include <string>
#include <map>

struct SConfig
{
    int working_time_span{ 1500 };  // in seconds
    int break_time_span{ 300 };     // in seconds

    int break_time_span_long{ 600 };// in seconds
    int long_break_interval{ 4 };
    bool use_long_break{ false };

    bool auto_start{ false };

    bool auto_loop{ false };
    int max_loops{ 3 };

    bool show_time_seconds{ false };

    // 0 means switch program state between running and pause
    // 1 means open option dialog
    int taskbar_dc_action{ 1 };

    bool play_sound{ true };
    int sound_id{ 0 };

    bool show_logo{ true };
};

enum class EProgramState
{
    PS_RUNNING,
    PS_PAUSED,
    PS_STOPPED,
};

enum class EPomodoroTimerState
{
    PTS_IN_WORK,
    PTS_SHORT_BREAK,
    PTS_LONG_BREAK,
};

class CDataManager
{
    CDataManager();
public:
    static CDataManager& Instance();

    int DPI(int pixel);
    float DPIF(float pixel);
    int RDPI(int pixel);

    const CString& StringRes(UINT id);

    SConfig& GetConfig();
    void LoadConfig(const std::wstring &cfg_dir);
    void SaveConfig() const;

    void StartPomodoroTimer();
    void PausePomodoroTimer();
    void StopPomodoroTimer();

    void SkipCurrentPomodoroTimerState();

    EProgramState GetProgramState() const;
    EPomodoroTimerState GetPomodoroTimerState() const;

    int GetRemaningTime() const;

    void Update();

    void PlaySoundById(int id) const;

private:
    bool NextLoop();

    static CDataManager m_instance;

    int m_dpi;
    SConfig m_config;

    EProgramState m_program_state;
    EPomodoroTimerState m_pt_state;

    std::wstring m_config_file_path;

    std::map<UINT, CString> m_string_res_map;
};
