#include "pch.h"
#include "Item.h"

const wchar_t* CPtItem::GetItemName() const
{
    // todo: use string resource
    return L"PomodoroTimer";
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
    return L"";
}

bool CPtItem::IsCustomDraw() const
{
    return true;
}

int CPtItem::GetItemWidthEx(void* hDC) const
{
    // todo
    return 0;
}

void CPtItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    // todo
}

int CPtItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag)
{
    // todo: show menu
    CWnd* pWnd = CWnd::FromHandle((HWND)hWnd);
    if (type == IPluginItem::MT_RCLICKED)
    {
        return 1;
    }

    return 0;
}
