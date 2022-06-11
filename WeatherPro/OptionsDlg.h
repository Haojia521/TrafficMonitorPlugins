#pragma once
#include "afxdialogex.h"
#include "DataAPI.h"

// COptionsDlg 对话框

class COptionsDlg : public CDialogEx
{
    DECLARE_DYNAMIC(COptionsDlg)

public:
    COptionsDlg(CWnd* pParent = nullptr);   // 标准构造函数
    virtual ~COptionsDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_OPTIONS_DLG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()
public:
    SCityInfo m_selected_city;
    CString m_currentCityName;

    BOOL m_showWeatherIcon;
    BOOL m_showWeatherInTooltip;
    BOOL m_showWeatherAlerts;
    BOOL m_showBriefWeatherAlertInfo;
    BOOL m_showBriefRTWeather;
    BOOL m_showErrorInfo;
    CComboBox m_ctrlInfoType;
    CComboBox m_ctrlDataApiType;

    afx_msg void OnBnClickedBtnSelectCity();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnBnClickedBtnUpdateManually();
    afx_msg void OnBnClickedBtnApiSettings();
};
