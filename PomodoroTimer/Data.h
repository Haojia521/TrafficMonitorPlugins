#pragma once

#include "Resource.h"

struct SConfig
{
    int working_time_span{ 1500 };  // in seconds
    int break_time_span{ 300 };     // in seconds

    bool auto_loop{ false };
    int max_loops{ 3 };

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
};

class CDataManager
{
    CDataManager();
public:
    static CDataManager& Instance();

    int DPI(int pixel);
    float DPIF(float pixel);
    int RDPI(int pixel);

    SConfig& GetConfig();

    void StartPomodoroTimer();
    void PausePomodoroTimer();
    void StopPomodoroTimer();

    void SkipCurrentPomodoroTimerState();

    EProgramState GetProgramState() const;
    EPomodoroTimerState GetPomodoroTimerState() const;

    int GetRemaningTime() const;

    void Update();
private:

    static CDataManager m_instance;

    int m_dpi;
    SConfig m_config;

    EProgramState m_program_state;
    EPomodoroTimerState m_pt_state;
};
