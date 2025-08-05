#pragma once
#include "afxdialogex.h"


// CPermanentItemsDlg 对话框

class CPermanentItemsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPermanentItemsDlg)

public:
	CPermanentItemsDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPermanentItemsDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PERMANENT_ITEMS_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_rt_humidity;
	BOOL m_rt_wind;
	BOOL m_rt_pm2p5;
	BOOL m_rt_pm10;
	BOOL m_rt_aqi;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
