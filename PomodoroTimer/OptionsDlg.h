#pragma once
#include "afxdialogex.h"


// COptionsDlg 对话框

class COptionsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(COptionsDlg)

public:
	COptionsDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~COptionsDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_OPTIONS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	void EnableControlsAboutLoops(BOOL enable = TRUE);
	void EnableControlsAboutSound(BOOL enable = TRUE);

	afx_msg void VerifyNumberEditValue(CEdit &edit, CSpinButtonCtrl &editBuddy);
public:
	virtual BOOL OnInitDialog();
    virtual void OnOK();
	afx_msg void OnEnChangeEditTimeSpanWork();
	afx_msg void OnEnChangeEditTimeSpanShortBreak();
	afx_msg void OnEnChangeEditNumLoops();
	afx_msg void OnBnClickedCheckAutoLoop();
	afx_msg void OnBnClickedCheckPlaySound();
	afx_msg void OnBnClickedBtnSoundTest();

	CEdit m_ctrlEditTimeSpanWork;
	CEdit m_ctrlEditTimeSpanShortBreak;
	CEdit m_ctrlEditNumLoops;
	CSpinButtonCtrl m_ctrlSpinTimeSpanWork;
	CSpinButtonCtrl m_ctrlSpinTimeSpanShortBreak;
	CSpinButtonCtrl m_ctrlSpinNumLoops;
	CComboBox m_ctrlSoundList;
	BOOL m_boolAutoStart;
	BOOL m_boolAutoLoop;
	BOOL m_boolPlaySound;
};
