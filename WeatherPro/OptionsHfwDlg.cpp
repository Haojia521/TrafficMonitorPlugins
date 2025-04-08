// OptionsHfwDlg.cpp: 实现文件
//

#include "pch.h"
#include "WeatherPro.h"
#include "afxdialogex.h"
#include "OptionsHfwDlg.h"

#include "DataManager.h"
#include "Common.h"

#include <filesystem>
#include <fstream>
#include <format>
#include <map>

// OptionsHfwDlg 对话框

IMPLEMENT_DYNAMIC(OptionsHfwDlg, CDialogEx)

OptionsHfwDlg::OptionsHfwDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_OPTIONS_HFW_DLG, pParent)
    , m_key(_T(""))
    , m_rt_temp_feels(FALSE)
    , m_rt_humidity(FALSE)
    , m_rt_wind(FALSE)
    , m_rt_wind_scale(FALSE)
    , m_aq(FALSE)
    , m_aq_aqi(FALSE)
    , m_aq_pm2p5(FALSE)
    , m_aq_pm10(FALSE)
    , m_fc_uv(FALSE)
    , m_fc_humidity(FALSE)
    , m_alert(FALSE)
    , m_api_host(_T(""))
    , m_project_id(_T(""))
    , m_credential_id(_T(""))
    , m_ssh_public_key_file(_T(""))
    , m_ssh_private_key_file(_T(""))
    , m_auth_by_key(FALSE)
{

}

OptionsHfwDlg::~OptionsHfwDlg()
{
}

void OptionsHfwDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_HFW_APP_KEY, m_key);
    DDX_Check(pDX, IDC_CHECK_HFW_RT_TEMP_FEELS, m_rt_temp_feels);
    DDX_Check(pDX, IDC_CHECK_HFW_RT_HUMIDITY, m_rt_humidity);
    DDX_Check(pDX, IDC_CHECK_HFW_RT_WIND, m_rt_wind);
    DDX_Check(pDX, IDC_CHECK_HFW_RT_WIND_SCALE, m_rt_wind_scale);
    DDX_Check(pDX, IDC_CHECK_HFW_AQ, m_aq);
    DDX_Check(pDX, IDC_CHECK_HFW_AQ_AQI, m_aq_aqi);
    DDX_Check(pDX, IDC_CHECK_HFW_AQ_PM2P5, m_aq_pm2p5);
    DDX_Check(pDX, IDC_CHECK_HFW_AQ_PM10, m_aq_pm10);
    DDX_Check(pDX, IDC_CHECK_HFW_FC_UV, m_fc_uv);
    DDX_Check(pDX, IDC_CHECK_HFW_FC_HUMIDITY, m_fc_humidity);
    DDX_Check(pDX, IDC_CHECK_HFW_ALERT, m_alert);
    DDX_Text(pDX, IDC_EDIT_HFW_API_HOST, m_api_host);
    DDX_Text(pDX, IDC_EDIT_HFW_SUB_ID, m_project_id);
    DDX_Text(pDX, IDC_EDIT_HFW_KEY_ID, m_credential_id);
    DDX_Text(pDX, IDC_STATIC_HFW_JWT_PUB_KEY_FILE, m_ssh_public_key_file);
    DDX_Text(pDX, IDC_STATIC_HFW_JWT_PRI_KEY_FILE, m_ssh_private_key_file);
    DDX_Radio(pDX, IDC_RADIO_HFW_AUTH_VIA_KEY, m_auth_by_key);
}


BEGIN_MESSAGE_MAP(OptionsHfwDlg, CDialogEx)
    ON_BN_CLICKED(IDC_RADIO_HFW_AUTH_VIA_KEY, &OptionsHfwDlg::OnBnClickedRadioHfwAuth)
    ON_BN_CLICKED(IDC_RADIO_HFW_AUTH_VIA_JWT, &OptionsHfwDlg::OnBnClickedRadioHfwAuth)
    ON_BN_CLICKED(IDC_BTN_HFW_JWT_CREATE_KEY_PAIR, &OptionsHfwDlg::OnBnClickedBtnHfwJwtCreateKeyPair)
    ON_BN_CLICKED(IDC_BTN_HFW_JWT_SELECT_KEY, &OptionsHfwDlg::OnBnClickedBtnHfwJwtSelectKey)
    ON_BN_CLICKED(IDC_BTN_HFW_JWT_COPY_PUB_KEY, &OptionsHfwDlg::OnBnClickedBtnHfwJwtCopyPubKey)
END_MESSAGE_MAP()

void OptionsHfwDlg::EnableControlsOfAuthorization(BOOL auth_by_key)
{	
    GetDlgItem(IDC_EDIT_HFW_APP_KEY)->EnableWindow(auth_by_key);
    GetDlgItem(IDC_EDIT_HFW_SUB_ID)->EnableWindow(!auth_by_key);
    GetDlgItem(IDC_EDIT_HFW_KEY_ID)->EnableWindow(!auth_by_key);
    GetDlgItem(IDC_BTN_HFW_JWT_CREATE_KEY_PAIR)->EnableWindow(!auth_by_key);
    GetDlgItem(IDC_BTN_HFW_JWT_SELECT_KEY)->EnableWindow(!auth_by_key);
    GetDlgItem(IDC_BTN_HFW_JWT_COPY_PUB_KEY)->EnableWindow(!auth_by_key);
    GetDlgItem(IDC_STATIC_HFW_JWT_PUB_KEY_FILE)->EnableWindow(!auth_by_key);
    GetDlgItem(IDC_STATIC_HFW_JWT_PRI_KEY_FILE)->EnableWindow(!auth_by_key);
}

// OptionsHfwDlg 消息处理程序

namespace detail
{
    std::map<int, int> tooltip_resources_id_pair{
        {IDC_STATIC_HFW_QUES_API_HOST,    IDS_HFW_QUES_API_HOST},
        {IDC_STATIC_HFW_QUES_JWT,         IDS_HFW_QUES_JWT},
        {IDC_STATIC_HFW_QUES_SUB_ID,      IDS_HFW_QUES_SUB_ID},
        {IDC_STATIC_HFW_QUES_JWT_KEY,     IDS_HFW_QUES_JWT_KEY_ID},
        {IDC_BTN_HFW_JWT_CREATE_KEY_PAIR, IDS_HFW_QUES_CREATE_KEY_PAIR},
        {IDC_BTN_HFW_JWT_SELECT_KEY,      IDS_HFW_QUES_SELECT_KEY},
        {IDC_BTN_HFW_JWT_COPY_PUB_KEY,    IDS_HFW_QUES_COPY_PUB_KEY},
    };
}

BOOL OptionsHfwDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    const auto &dm = CDataManager::Instance();

    for (const auto& pair : detail::tooltip_resources_id_pair) {
        int ctrl_id = pair.first;      // 控件ID（如 IDC_STATIC1）
        int str_id = pair.second;      // 字符串资源ID（如 IDS_TOOLTIP1）

        // 动态创建 ToolTip
        auto pToolTip = std::make_unique<CToolTipCtrl>();
        pToolTip->Create(this);
        pToolTip->AddTool(GetDlgItem(ctrl_id), dm.StringRes(str_id));
        pToolTip->Activate(TRUE);
        m_toolTips.push_back(std::move(pToolTip));
    }

    if (m_api != nullptr)
    {
        const auto &config = m_api->config;

        m_key = config.AppKey.c_str();

        m_api_host = config.ApiHost.c_str();
        m_project_id = config.ProjectID.c_str();
        m_credential_id = config.CredentialID.c_str();
        m_ssh_public_key_file = config.JwtPublicKeyFile.c_str();
        m_ssh_private_key_file = config.JwtPrivateKeyFile.c_str();
        m_auth_by_key = config.AuthViaJWT ? 1 : 0;
        EnableControlsOfAuthorization(m_auth_by_key == 0 ? TRUE : FALSE);

        m_rt_temp_feels = config.ShowRealtimeTemperatureFeelsLike ? TRUE : FALSE;
        m_rt_humidity = config.ShowRealtimeHumidity ? TRUE : FALSE;
        m_rt_wind = config.ShowRealtimeWind ? TRUE : FALSE;
        m_rt_wind_scale = config.ShowRealtimeWindScale ? TRUE : FALSE;
        m_aq = config.ShowAirQuality ? TRUE : FALSE;
        m_aq_aqi = config.ShowAirQualityAQI ? TRUE : FALSE;
        m_aq_pm2p5 = config.ShowAirQualityPM2p5 ? TRUE : FALSE;
        m_aq_pm10 = config.ShowAirQualityPM10 ? TRUE : FALSE;
        m_fc_uv = config.ShowForecastUVIdex ? TRUE : FALSE;
        m_fc_humidity = config.showForecastHumidity ? TRUE : FALSE;
        m_alert = config.ShowWeatherAlert ? TRUE : FALSE;

        UpdateData(FALSE);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}


void OptionsHfwDlg::OnOK()
{
    if (m_api != nullptr)
    {
        UpdateData(TRUE);

        auto &config = m_api->config;
        config.AppKey = m_key;

        config.ApiHost = m_api_host;
        config.ProjectID = m_project_id;
        config.CredentialID = m_credential_id;
        config.JwtPublicKeyFile = m_ssh_public_key_file;
        config.JwtPrivateKeyFile = m_ssh_private_key_file;
        config.AuthViaJWT = m_auth_by_key == 1;

        config.ShowRealtimeTemperatureFeelsLike = m_rt_temp_feels == TRUE;
        config.ShowRealtimeWind = m_rt_wind == TRUE;
        config.ShowRealtimeWindScale = m_rt_wind_scale == TRUE;
        config.ShowRealtimeHumidity = m_rt_humidity == TRUE;
        config.ShowForecastUVIdex = m_fc_uv == TRUE;
        config.showForecastHumidity = m_fc_humidity == TRUE;
        config.ShowAirQuality = m_aq == TRUE;
        config.ShowAirQualityAQI = m_aq_aqi == TRUE;
        config.ShowAirQualityPM2p5 = m_aq_pm2p5 == TRUE;
        config.ShowAirQualityPM10 = m_aq_pm10 == TRUE;
        config.ShowWeatherAlert = m_alert == TRUE;

        CDataManager::InstanceRef().RefreshWeatherInfoCache();
        CWeatherPro::Instance().UpdateTooltip(CDataManager::Instance().GetTooptipInfo());

        CDataManager::Instance().SaveConfigs();
    }

    CDialogEx::OnOK();
}

void OptionsHfwDlg::OnBnClickedRadioHfwAuth()
{
    UpdateData(true);

    EnableControlsOfAuthorization(m_auth_by_key == 0 ? TRUE : FALSE);
}

CString SelectFolder()
{
    CString strFolderPath = _T("");

    CComPtr<IFileDialog> pFileDialog;
    HRESULT hr = pFileDialog.CoCreateInstance(CLSID_FileOpenDialog);

    if (SUCCEEDED(hr))
    {
        // 设置选项 - 只选择文件夹
        DWORD dwOptions;
        pFileDialog->GetOptions(&dwOptions);
        pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

        // 显示对话框
        hr = pFileDialog->Show(nullptr);

        if (SUCCEEDED(hr))
        {
            CComPtr<IShellItem> pItem;
            hr = pFileDialog->GetResult(&pItem);

            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                if (SUCCEEDED(hr))
                {
                    strFolderPath = CString(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
            }
        }
    }

    return strFolderPath;
}


void OptionsHfwDlg::OnBnClickedBtnHfwJwtCreateKeyPair()
{
    CString dir_path = SelectFolder();

    if (dir_path.IsEmpty()) {
        return;
    }

    std::vector<std::string> errors;
    auto [pri_key, pub_key] = utils::generate_ed25519_keypair(errors);

    std::wstring pri_key_file = std::format(L"{}\\hfw-ed25519-private.pem", dir_path.GetString());
    std::wstring pub_key_file = std::format(L"{}\\hfw-ed25519-public.pem", dir_path.GetString());

    bool generate_keypair{ true };

    const auto &dm = CDataManager::Instance();

    if (std::filesystem::exists(pri_key_file) && std::filesystem::exists(pub_key_file)) {
        auto ret = ::MessageBox(GetSafeHwnd(),
                                dm.StringRes(IDS_HFW_MB_CKP_KEY_EXISTED), dm.StringRes(IDS_HFW_MB_TITLE_CREATE_KEYPAIR),
                                MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION);
        if (ret == IDCANCEL) {
            return;
        } else if (ret == IDNO) {
            generate_keypair = false;
        }
    }

    if (generate_keypair) {
        std::ofstream ofs_pri(pri_key_file);
        if (ofs_pri.is_open()) {
            ofs_pri << pri_key;
            ofs_pri.close();
        } else {
            ::MessageBox(GetSafeHwnd(),
                         dm.StringRes(IDS_HFW_MB_CKP_PRI_FAILED), dm.StringRes(IDS_HFW_MB_TITLE_CREATE_KEYPAIR),
                         MB_ICONERROR);
            return;
        }

        std::ofstream ofs_pub(pub_key_file);
        if (ofs_pub.is_open()) {
            ofs_pub << pub_key;
            ofs_pub.close();
        } else {
            ::MessageBox(GetSafeHwnd(),
                         dm.StringRes(IDS_HFW_MB_CKP_PUB_FAILED), dm.StringRes(IDS_HFW_MB_TITLE_CREATE_KEYPAIR),
                         MB_ICONERROR);
            return;
        }
    }

    m_ssh_private_key_file = pri_key_file.c_str();
    m_ssh_public_key_file = pub_key_file.c_str();

    UpdateData(FALSE);
}

void OptionsHfwDlg::OnBnClickedBtnHfwJwtSelectKey()
{
    CString dir_path = SelectFolder();

    if (dir_path.IsEmpty()) {
        return;
    }

    std::wstring pri_key_file = std::format(L"{}\\hfw-ed25519-private.pem", dir_path.GetString());
    std::wstring pub_key_file = std::format(L"{}\\hfw-ed25519-public.pem", dir_path.GetString());

    namespace fs = std::filesystem;
    if (fs::exists(pri_key_file) && fs::exists(pub_key_file)) {
        m_ssh_private_key_file = pri_key_file.c_str();
        m_ssh_public_key_file = pub_key_file.c_str();

        UpdateData(FALSE);
    } else {
        const auto &dm = CDataManager::Instance();
        ::MessageBox(GetSafeHwnd(),
                     dm.StringRes(IDS_HFW_MB_SKP_NOT_FOUND), dm.StringRes(IDS_HFW_MB_TITLE_SELECT_KEY),
                     MB_ICONINFORMATION);
    }
}

bool CopyStringToClipboard(const CString& strText, CWnd* pWnd /*= nullptr*/)
{
    // 默认父窗口为空时，可传入AfxGetMainWnd()或其他有效窗口指针
    CWnd* pOwner = (pWnd != nullptr) ? pWnd : AfxGetMainWnd();
    if (!pOwner)
        return false; // 确保有一个窗口可以关联

    // 1) 打开剪贴板，如果失败则返回
    if (!::OpenClipboard(pOwner->GetSafeHwnd()))
        return false;

    // 2) 清空剪贴板
    ::EmptyClipboard();

    // 计算内存大小（含终止符）
#ifdef _UNICODE
    // Unicode 情况下，使用CF_UNICODETEXT，每个字符2字节
    SIZE_T nSize = (strText.GetLength() + 1) * sizeof(wchar_t);
    UINT uFormat = CF_UNICODETEXT;
#else
    // ANSI情况下，使用CF_TEXT
    SIZE_T nSize = (strText.GetLength() + 1) * sizeof(char);
    UINT uFormat = CF_TEXT;
#endif

    // 3) 分配可移动的全局内存
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
    if (!hGlobal)
    {
        ::CloseClipboard();
        return false;
    }

    // 4) 将字符串内容复制到刚分配的内存中
    LPVOID pData = ::GlobalLock(hGlobal);
    if (!pData)
    {
        ::GlobalFree(hGlobal);
        ::CloseClipboard();
        return false;
    }
    // 将 strText 的内容拷贝进去（包括末尾的 '\0'）
    memcpy(pData, (LPCTSTR)strText, nSize);
    ::GlobalUnlock(hGlobal);

    // 5) 设置剪贴板数据并关闭
    ::SetClipboardData(uFormat, hGlobal);
    // 注意：SetClipboardData 成功后，系统会接管 hGlobal 的内存管理，
    // 无需再手动 GlobalFree。如果失败了则需要你自己释放。

    ::CloseClipboard();

    return true;
}

void OptionsHfwDlg::OnBnClickedBtnHfwJwtCopyPubKey()
{
    const auto &dm = CDataManager::Instance();

    if (std::filesystem::exists(m_ssh_public_key_file.GetString())) {
        std::wifstream ifs(m_ssh_public_key_file.GetString());

        if (ifs.is_open()) {
            std::wstring pub_key{ std::istreambuf_iterator<wchar_t>(ifs), std::istreambuf_iterator<wchar_t>() };

            if (CopyStringToClipboard(pub_key.c_str(), this)) {
                ::MessageBox(GetSafeHwnd(),
                             dm.StringRes(IDS_HFW_MB_CPK_COPIED), dm.StringRes(IDS_HFW_MB_TITLE_COPY_PUB_KEY),
                             MB_ICONINFORMATION);
            } else {
                ::MessageBox(GetSafeHwnd(),
                             dm.StringRes(IDS_HFW_MB_CPK_FAILED), dm.StringRes(IDS_HFW_MB_TITLE_COPY_PUB_KEY),
                             MB_ICONWARNING);
            }

            ifs.close();
        } else {
            ::MessageBox(GetSafeHwnd(),
                         dm.StringRes(IDS_HFW_MB_CPK_CANNOT_OPEN), dm.StringRes(IDS_HFW_MB_TITLE_COPY_PUB_KEY),
                         MB_ICONWARNING);
        }
    } else {
        ::MessageBox(GetSafeHwnd(),
                     dm.StringRes(IDS_HFW_MB_CPK_NOT_FOUND), dm.StringRes(IDS_HFW_MB_TITLE_COPY_PUB_KEY),
                     MB_ICONWARNING);
    }
}

BOOL OptionsHfwDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST) {
        for (auto& toolTip : m_toolTips) {
            if (toolTip->GetSafeHwnd()) {
                toolTip->RelayEvent(pMsg); // 转发消息给所有 ToolTip
            }
        }
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}
