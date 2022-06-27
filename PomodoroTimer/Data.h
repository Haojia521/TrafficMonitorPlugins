#pragma once

#include "Resource.h"

struct SConfig
{
    int working_time_span{ 1500 };  // in seconds
    int break_time_span{ 300 };     // in seconds

    bool auto_loop{ false };
    int loop_times{ 3 };

    int sound_id{ 0 };

    bool show_logo{ true };
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

private:
    static CDataManager m_instance;

    int m_dpi;
    SConfig m_config;
};
