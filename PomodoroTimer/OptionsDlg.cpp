// OptionsDlg.cpp: 实现文件
//

#include "pch.h"
#include "PomodoroTimer.h"
#include "afxdialogex.h"
#include "OptionsDlg.h"

#include "Data.h"


// COptionsDlg 对话框

IMPLEMENT_DYNAMIC(COptionsDlg, CDialogEx)

COptionsDlg::COptionsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_OPTIONS, pParent)
	, m_boolAutoLoop(FALSE)
{

}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPIN_TIME_SPAN_WORK, m_ctrlSpinTimeSpanWork);
	DDX_Control(pDX, IDC_SPIN_TIME_SPAN_SHORT_BREAK, m_ctrlSpinTimeSpanShortBreak);
	DDX_Control(pDX, IDC_SPIN_NUM_LOOPS, m_ctrlSpinNumLoops);
	DDX_Check(pDX, IDC_CHECK_AUTO_LOOP, m_boolAutoLoop);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_AUTO_LOOP, &COptionsDlg::OnBnClickedCheckAutoLoop)
END_MESSAGE_MAP()


// COptionsDlg 消息处理程序


BOOL COptionsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	auto &data_manager = CDataManager::Instance();
	const auto &cfg = data_manager.GetConfig();

	auto time_span_work = cfg.working_time_span / 60;
	if (time_span_work < 1) time_span_work = 1;

	auto time_span_short_break = cfg.break_time_span / 60;
	if (time_span_short_break < 1) time_span_short_break = 1;

	// init spin controls
	m_ctrlSpinTimeSpanWork.SetRange(1, 240);
	m_ctrlSpinTimeSpanWork.SetPos(time_span_work);

	m_ctrlSpinTimeSpanShortBreak.SetRange(1, 240);
	m_ctrlSpinTimeSpanShortBreak.SetPos(time_span_short_break);

	m_ctrlSpinNumLoops.SetRange(1, 10);
	m_ctrlSpinNumLoops.SetPos(cfg.max_loops);

	// enable/disenable controls about loops
	EnableControlsAboutLoops(cfg.auto_loop ? TRUE : FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void COptionsDlg::EnableControlsAboutLoops(BOOL enable /* = TRUE */)
{
    GetDlgItem(IDC_EDIT_NUM_LOOPS)->EnableWindow(enable);
    GetDlgItem(IDC_SPIN_NUM_LOOPS)->EnableWindow(enable);
}


void COptionsDlg::OnOK()
{
	// cannot update options if timer is still running
	if (CDataManager::Instance().GetProgramState() != EProgramState::PS_STOPPED)
	{
		MessageBox(L"番茄钟正在运行，无法保存设置", L"选项");
		return;
	}

	UpdateData(TRUE);

	auto &cfg = CDataManager::Instance().GetConfig();

	cfg.working_time_span = m_ctrlSpinTimeSpanWork.GetPos() * 60;
	cfg.break_time_span = m_ctrlSpinTimeSpanShortBreak.GetPos() * 60;

	cfg.auto_loop = m_boolAutoLoop == TRUE;
	cfg.max_loops = m_ctrlSpinNumLoops.GetPos();

	// todo: complete other options
	// todo: save config to file

	CDialogEx::OnOK();
}


void COptionsDlg::OnBnClickedCheckAutoLoop()
{
	UpdateData(TRUE);

	EnableControlsAboutLoops(m_boolAutoLoop);
}
