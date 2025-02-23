#include "pch.h"
#include "WeatherPro.h"
#include "WeatherProItem.h"
#include "DataManager.h"
#include <algorithm>
#include <gdiplus.h>

namespace item
{
    static HBITMAP LoadPngFromResource(UINT resourceID) {
        HBITMAP hBitmap{ nullptr };
        HINSTANCE hInstance = AfxGetInstanceHandle();
        HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), _T("PNG"));
        if (hResource) {
            HGLOBAL hMemory = LoadResource(hInstance, hResource);
            if (hMemory) {
                LPVOID pResourceData = LockResource(hMemory);
                DWORD resourceSize = SizeofResource(hInstance, hResource);

                // 创建 GDI+ Bitmap 从内存
                IStream* pStream = nullptr;
                if (CreateStreamOnHGlobal(nullptr, TRUE, &pStream) == S_OK) {
                    pStream->Write(pResourceData, resourceSize, nullptr);
                    Gdiplus::Bitmap gdiplusBitmap(pStream);
                    if (gdiplusBitmap.GetLastStatus() == Gdiplus::Ok) {
                        gdiplusBitmap.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hBitmap);
                    }
                    pStream->Release();
                }
            }
        }
        return hBitmap;
    }

    class MemoryDcHelper
    {
    public:
        MemoryDcHelper(CDC *dc) {
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            bitmap_.Attach(LoadPngFromResource(IDB_PNG_LOADING_FRAMES));

            memory_dc_.CreateCompatibleDC(dc);
            memory_dc_.SelectObject(&bitmap_);
        }

        ~MemoryDcHelper() {
            memory_dc_.SelectObject((CBitmap *)nullptr);
        }

        CDC* DC() {
            return &memory_dc_;
        }
    private:
        CBitmap bitmap_;
        CDC memory_dc_;
    };

    static void DrawLoadingFrames(CDC *dc, int x, int y) {
        static MemoryDcHelper helper(dc);
        static int idx{ 0 };
        const int rows{ 2 }, cols{ 8 }, frame_size{ 16 };

        int r = idx / cols;
        int c = idx % cols;

        int src_x = c * frame_size;
        int src_y = r * frame_size;

        dc->BitBlt(x, y, frame_size, frame_size, helper.DC(), src_x, src_y, SRCCOPY);

        idx = (idx + 1) % (rows * cols);
    }
}

const wchar_t* CWeatherProItem::GetItemName() const
{
    return CDataManager::Instance().StringRes(IDS_WEATHER_PRO);
}

const wchar_t* CWeatherProItem::GetItemId() const
{
    return L"FxOP34Lm";
}

const wchar_t* CWeatherProItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CWeatherProItem::GetItemValueText() const
{
    return L"";
}

const wchar_t* CWeatherProItem::GetItemValueSampleText() const
{
    const auto &config = CDataManager::Instance().GetConfig();
    if (config.m_show_weather_icon)
    {
        if (config.m_wit == EWeatherInfoType::WEATHER_REALTIME)
            return L"20℃";
        else
            return L"20℃~20℃";
    }
    else
    {
        if (config.m_wit == EWeatherInfoType::WEATHER_REALTIME)
            return L"多云 20℃";
        else
            return L"多云转晴 20℃~20℃";
    }
    return L"";
}

bool CWeatherProItem::IsCustomDraw() const
{
    return true;
}

int CWeatherProItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    auto icon_width = CDataManager::Instance().GetConfig().m_show_weather_icon ? CDataManager::Instance().DPI(20) : 0;

    return icon_width + std::max(pDC->GetTextExtent(CDataManager::Instance().GetWeatherTemperature().c_str()).cx,
                                 pDC->GetTextExtent(GetItemValueSampleText()).cx);
}

void CWeatherProItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    CRect rect(CPoint(x, y), CSize(w, h));

    const auto &dm = CDataManager::Instance();
    auto is_updating = dm.IsUpdating();

    if (dm.GetConfig().m_show_weather_icon)
    {
        auto icon_size = dm.DPI(16);
        auto hIcon = CDataManager::InstanceRef().GetIcon();
        CPoint icon_pos{ rect.TopLeft() };
        icon_pos.x += dm.DPI(2);
        icon_pos.y += (rect.Height() - icon_size) / 2;

        if (is_updating) {
            item::DrawLoadingFrames(pDC, icon_pos.x, icon_pos.y);
        } else {
            DrawIconEx(pDC->GetSafeHdc(), icon_pos.x, icon_pos.y, hIcon, icon_size, icon_size, 0, NULL, DI_NORMAL);
        }

        rect.left += dm.DPI(20);
    }

    if (is_updating) {
        pDC->DrawText(L"更新中", rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        pDC->DrawText(dm.GetWeatherTemperature().c_str(), rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }
}

int CWeatherProItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag)
{
    if (type == IPluginItem::MT_DBCLICKED)
    {
        const auto &dm = CDataManager::Instance();

        if (dm.GetConfig().m_double_click_action == 0)
            CWeatherPro::Instance().ShowOptionsDialog(hWnd);
        else
            CWeatherPro::Instance().UpdateWeatherInfo(true);

        return 1;
    }

    return 0;
}
