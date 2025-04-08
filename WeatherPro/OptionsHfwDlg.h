#pragma once
#include "afxdialogex.h"
#include "DataApiHefengWeather.h"

#include <vector>


// OptionsHfwDlg 对话框

class OptionsHfwDlg : public CDialogEx
{
	DECLARE_DYNAMIC(OptionsHfwDlg)

public:
	OptionsHfwDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~OptionsHfwDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OPTIONS_HFW_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	void EnableControlsOfAuthorization(BOOL auth_by_key);

	std::vector<std::unique_ptr<CToolTipCtrl>> m_toolTips;
public:
	CString m_key;
	CString m_api_host;
	CString m_project_id;
	CString m_credential_id;
	CString m_ssh_public_key_file;
	CString m_ssh_private_key_file;
	BOOL m_auth_by_key;
	BOOL m_rt_temp_feels;
	BOOL m_rt_humidity;
	BOOL m_rt_wind;
	BOOL m_rt_wind_scale;
	BOOL m_aq;
	BOOL m_aq_aqi;
	BOOL m_aq_pm2p5;
	BOOL m_aq_pm10;
	BOOL m_fc_uv;
	BOOL m_fc_humidity;
    BOOL m_alert;

	std::shared_ptr<DataApiHefengWeather> m_api;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedRadioHfwAuth();
	afx_msg void OnBnClickedBtnHfwJwtCreateKeyPair();
	afx_msg void OnBnClickedBtnHfwJwtSelectKey();
	afx_msg void OnBnClickedBtnHfwJwtCopyPubKey();
};
