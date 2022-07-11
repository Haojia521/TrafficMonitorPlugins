#include "pch.h"
#include "Item.h"
#include "Data.h"
#include "PomodoroTimer.h"

#include <string>
#include <map>

namespace item
{
    class LogoHelper
    {
    public:
        LogoHelper()
        {
            size = CDataManager::Instance().DPI(16);
        }

        HICON get_logo()
        {
            auto &dm = CDataManager::Instance();

            if (dm.GetProgramState() == EProgramState::PS_STOPPED)
                return _get_logo_by_id(IDI_ICON_LOGO_STOP);
            else
            {
                if (dm.GetPomodoroTimerState() == EPomodoroTimerState::PTS_IN_WORK)
                    return _get_logo_by_id(IDI_ICON_LOGO);
                else
                    return _get_logo_by_id(IDI_ICON_LOGO_BREAK);
            }
        }

    private:
        HICON _get_logo_by_id(int id)
        {
            if (_icon_map.count(id) == 0)
            {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());
                _icon_map[id] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, size, size, 0);
            }

            return _icon_map[id];
        }

        int size;
        std::map<int, HICON> _icon_map;
    };

    LogoHelper logo_helper;

    std::wstring get_label_text()
    {
        auto &data_manager = CDataManager::Instance();
        auto prog_state = data_manager.GetProgramState();
        auto pt_state = data_manager.GetPomodoroTimerState();
        const auto &cfg = data_manager.GetConfig();

        if (prog_state == EProgramState::PS_RUNNING)
        {
            auto t = data_manager.GetRemaningTime();
            std::wstring str;

            if (!cfg.show_logo)
            {
                if (pt_state == EPomodoroTimerState::PTS_IN_WORK)
                    str += data_manager.StringRes(IDS_PTS_WORK).GetString();
                else
                    str += data_manager.StringRes(IDS_PTS_BREAK).GetString();
            }

            if (!cfg.show_time_seconds)
            {
                int target_time = 0;
                if (pt_state == EPomodoroTimerState::PTS_IN_WORK)
                    target_time = cfg.working_time_span;
                else
                    target_time = cfg.break_time_span;

                int m = t / 60;
                if (t != target_time)
                    m += 1;

                str += std::to_wstring(m) + data_manager.StringRes(IDS_MINUTE).GetString();
            }
            else
            {
                int m = t / 60;
                int s = t % 60;

                wchar_t buffer[10]{ 0 };
                swprintf_s(buffer, L"%02d:%02d", m, s);

                str += buffer;
            }
            return str;
        }
        else if (prog_state == EProgramState::PS_PAUSED)
            return data_manager.StringRes(IDS_PS_PAUSE).GetString();
        else
            return data_manager.StringRes(IDS_PS_STOP).GetString();
    }
}

const wchar_t* CPtItem::GetItemName() const
{
    return CDataManager::Instance().StringRes(IDS_PLUGIN_NAME).GetString();
}

const wchar_t* CPtItem::GetItemId() const
{
    return L"znj5bTOB";
}

const wchar_t* CPtItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CPtItem::GetItemValueText() const
{
    return L"";
}

const wchar_t* CPtItem::GetItemValueSampleText() const
{
    const auto &cfg = CDataManager::Instance().GetConfig();

    if (cfg.show_logo)
    {
        if (cfg.show_time_seconds)
            return L"000:00";
        else
            return L"25分钟";
    }
    else
    {
        if (cfg.show_time_seconds)
            return L"工作中 000:00";
        else
            return L"工作中 25分钟";
    }
}

bool CPtItem::IsCustomDraw() const
{
    return true;
}

int CPtItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    auto icon_width = CDataManager::Instance().GetConfig().show_logo ? CDataManager::Instance().DPI(20) : 0;
    icon_width += pDC->GetTextExtent(GetItemValueSampleText()).cx;

    return icon_width;
}

void CPtItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    CRect rect(CPoint(x, y), CSize(w, h));
    if (CDataManager::Instance().GetConfig().show_logo)
    {
        auto icon_size = CDataManager::Instance().DPI(16);
        auto logo_icon = item::logo_helper.get_logo();

        CPoint icon_pos{ rect.TopLeft() };
        icon_pos.x += CDataManager::Instance().DPI(2);
        icon_pos.y += (rect.Height() - icon_size) / 2;
        DrawIconEx(pDC->GetSafeHdc(), icon_pos.x, icon_pos.y, logo_icon, icon_size, icon_size, 0, NULL, DI_NORMAL);

        rect.left += CDataManager::Instance().DPI(20);
    }

    pDC->DrawText(item::get_label_text().c_str(), rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

int CPtItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag)
{
    CWnd* pWnd = CWnd::FromHandle((HWND)hWnd);
    if (type == IPluginItem::MT_RCLICKED)
    {
        CPomodoroTimer::Instance().ShowContextMenu(pWnd);
        return 1;
    }
    else if (type == IPluginItem::MT_DBCLICKED)
    {
        auto &data_manager = CDataManager::Instance();

        if (data_manager.GetConfig().taskbar_dc_action == 0)
        {
            if (data_manager.GetProgramState() == EProgramState::PS_RUNNING)
                data_manager.PausePomodoroTimer();
            else
                data_manager.StartPomodoroTimer();
        }
        else
            CPomodoroTimer::Instance().ShowOptionsDialog();

        return 1;
    }

    return 0;
}
