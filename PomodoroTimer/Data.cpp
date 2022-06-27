#include "pch.h"
#include "Data.h"

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
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
