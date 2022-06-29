#include "pch.h"
#include "Item.h"
#include "Data.h"

namespace item
{
    HICON _logo = nullptr;

    HICON get_logo()
    {
        if (_logo == nullptr)
        {
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            _logo = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_LOGO), IMAGE_ICON,
                CDataManager::Instance().DPI(16), CDataManager::Instance().DPI(16), 0);
        }

        return _logo;
    }
}

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
    return L"工作中 (25min)";
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
    // todo
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    CRect rect(CPoint(x, y), CSize(w, h));
    if (CDataManager::Instance().GetConfig().show_logo)
    {
        auto icon_size = CDataManager::Instance().DPI(16);
        auto logo_icon = item::get_logo();

        CPoint icon_pos{ rect.TopLeft() };
        icon_pos.x += CDataManager::Instance().DPI(2);
        icon_pos.y += (rect.Height() - icon_size) / 2;
        DrawIconEx(pDC->GetSafeHdc(), icon_pos.x, icon_pos.y, logo_icon, icon_size, icon_size, 0, NULL, DI_NORMAL);

        rect.left += CDataManager::Instance().DPI(20);
    }

    // todo: get real state text from data manager
    pDC->DrawText(GetItemValueSampleText(), rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
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
