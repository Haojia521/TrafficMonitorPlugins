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
	, m_boolPlaySound(FALSE)
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
	DDX_Check(pDX, IDC_CHECK_PLAY_SOUND, m_boolPlaySound);
	DDX_Control(pDX, IDC_COMBO_SOUND_LIST, m_ctrlSoundList);
	DDX_Control(pDX, IDC_EDIT_TIME_SPAN_WORK, m_ctrlEditTimeSpanWork);
	DDX_Control(pDX, IDC_EDIT_TIME_SPAN_SHORT_BREAK, m_ctrlEditTimeSpanShortBreak);
	DDX_Control(pDX, IDC_EDIT_NUM_LOOPS, m_ctrlEditNumLoops);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_AUTO_LOOP, &COptionsDlg::OnBnClickedCheckAutoLoop)
	ON_BN_CLICKED(IDC_CHECK_PLAY_SOUND, &COptionsDlg::OnBnClickedCheckPlaySound)
	ON_BN_CLICKED(IDC_BTN_SOUND_TEST, &COptionsDlg::OnBnClickedBtnSoundTest)
	ON_EN_CHANGE(IDC_EDIT_TIME_SPAN_WORK, &COptionsDlg::OnEnChangeEditTimeSpanWork)
	ON_EN_CHANGE(IDC_EDIT_TIME_SPAN_SHORT_BREAK, &COptionsDlg::OnEnChangeEditTimeSpanShortBreak)
	ON_EN_CHANGE(IDC_EDIT_NUM_LOOPS, &COptionsDlg::OnEnChangeEditNumLoops)
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

	// enable/disable controls about loops
	m_boolAutoLoop = cfg.auto_loop ? TRUE : FALSE;
	EnableControlsAboutLoops(m_boolAutoLoop);

	// enable/disable controls about sound
	m_boolPlaySound = cfg.play_sound ? TRUE : FALSE;
	EnableControlsAboutSound(m_boolPlaySound);

	// init sound list combobox
	m_ctrlSoundList.AddString(L"Sound-1");
	m_ctrlSoundList.AddString(L"Sound-2");
	m_ctrlSoundList.AddString(L"Sound-3");
	m_ctrlSoundList.SetCurSel(cfg.sound_id);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void COptionsDlg::EnableControlsAboutLoops(BOOL enable /* = TRUE */)
{
    GetDlgItem(IDC_EDIT_NUM_LOOPS)->EnableWindow(enable);
    GetDlgItem(IDC_SPIN_NUM_LOOPS)->EnableWindow(enable);
}

void COptionsDlg::EnableControlsAboutSound(BOOL enable /* = TRUE */)
{
	GetDlgItem(IDC_COMBO_SOUND_LIST)->EnableWindow(enable);
	GetDlgItem(IDC_BTN_SOUND_TEST)->EnableWindow(enable);
}


void COptionsDlg::OnOK()
{
	auto &data_manager = CDataManager::Instance();

	// cannot update options if timer is still running
	if (data_manager.GetProgramState() != EProgramState::PS_STOPPED)
	{
		MessageBox(data_manager.StringRes(IDS_OPT_DLG_CANNOT_SAVE),
			data_manager.StringRes(IDS_OPT_DLG_TITLE),
			MB_OK | MB_ICONERROR);
		return;
	}

	UpdateData(TRUE);

	auto &cfg = CDataManager::Instance().GetConfig();

	cfg.working_time_span = m_ctrlSpinTimeSpanWork.GetPos() * 60;
	cfg.break_time_span = m_ctrlSpinTimeSpanShortBreak.GetPos() * 60;

	cfg.auto_loop = m_boolAutoLoop == TRUE;
	cfg.max_loops = m_ctrlSpinNumLoops.GetPos();

	cfg.play_sound = m_boolPlaySound == TRUE;
	cfg.sound_id = m_ctrlSoundList.GetCurSel();

	CDataManager::Instance().SaveConfig();

	CDialogEx::OnOK();
}


void COptionsDlg::OnBnClickedCheckAutoLoop()
{
	UpdateData(TRUE);

	EnableControlsAboutLoops(m_boolAutoLoop);
}


void COptionsDlg::OnBnClickedCheckPlaySound()
{
	UpdateData(TRUE);

	EnableControlsAboutSound(m_boolPlaySound);
}


void COptionsDlg::OnBnClickedBtnSoundTest()
{
	CDataManager::Instance().PlaySoundById(m_ctrlSoundList.GetCurSel());
}


void COptionsDlg::VerifyNumberEditValue(CEdit &edit, CSpinButtonCtrl &editBuddy)
{
    BOOL err{ FALSE };
    auto pos = editBuddy.GetPos32(&err);

	if (err)
	{
		editBuddy.SetPos(pos);
		edit.SetSel(0, -1);
	}
}


void COptionsDlg::OnEnChangeEditTimeSpanWork()
{
	VerifyNumberEditValue(m_ctrlEditTimeSpanWork, m_ctrlSpinTimeSpanWork);
}


void COptionsDlg::OnEnChangeEditTimeSpanShortBreak()
{
	VerifyNumberEditValue(m_ctrlEditTimeSpanShortBreak, m_ctrlSpinTimeSpanShortBreak);
}


void COptionsDlg::OnEnChangeEditNumLoops()
{
	VerifyNumberEditValue(m_ctrlEditNumLoops, m_ctrlSpinNumLoops);
}
