// OptionsDlg.cpp: 实现文件
//

#include "pch.h"
#include "PomodoroTimer.h"
#include "afxdialogex.h"
#include "OptionsDlg.h"

#include "Data.h"


// COptionsDlg 对话框

IMPLEMENT_DYNAMIC(COptionsDlg, CDialogEx)

CWnd* COptionsDlg::m_pInstance = nullptr;

COptionsDlg::COptionsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_OPTIONS, pParent)
	, m_boolAutoLoop(FALSE)
	, m_boolPlaySound(FALSE)
	, m_boolAutoStart(FALSE)
	, m_boolShowSeconds(FALSE)
	, m_intRadioDoubleClickAction(0)
	, m_boolUseLongBreak(FALSE)
{
	m_pInstance = this;
}

COptionsDlg::~COptionsDlg()
{
	m_pInstance = nullptr;
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
	DDX_Check(pDX, IDC_CHECK_AUTO_START, m_boolAutoStart);
	DDX_Check(pDX, IDC_CHECK_SHOW_SECONDS, m_boolShowSeconds);
	DDX_Radio(pDX, IDC_RADIO_DC_SWITCH_TIMER_STATE, m_intRadioDoubleClickAction);
	DDX_Check(pDX, IDC_CHECK_USE_LONG_BREAK, m_boolUseLongBreak);
	DDX_Control(pDX, IDC_EDIT_TIME_SPAN_LONG_BREAK, m_ctrlEditTimeSpanLongBreak);
	DDX_Control(pDX, IDC_SPIN_TIME_SPAN_LONG_BREAK, m_ctrlSpinTimeSpanLongBreak);
	DDX_Control(pDX, IDC_EDIT_LONG_BREAK_INTERVAL, m_ctrlEditLongBreakInterval);
	DDX_Control(pDX, IDC_SPIN_LONG_BREAK_INTERVAL, m_ctrlSpinLongBreakInterval);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_AUTO_LOOP, &COptionsDlg::OnBnClickedCheckAutoLoop)
	ON_BN_CLICKED(IDC_CHECK_PLAY_SOUND, &COptionsDlg::OnBnClickedCheckPlaySound)
	ON_BN_CLICKED(IDC_BTN_SOUND_TEST, &COptionsDlg::OnBnClickedBtnSoundTest)
	ON_EN_CHANGE(IDC_EDIT_TIME_SPAN_WORK, &COptionsDlg::OnEnChangeEditTimeSpanWork)
	ON_EN_CHANGE(IDC_EDIT_TIME_SPAN_SHORT_BREAK, &COptionsDlg::OnEnChangeEditTimeSpanShortBreak)
	ON_EN_CHANGE(IDC_EDIT_NUM_LOOPS, &COptionsDlg::OnEnChangeEditNumLoops)
	ON_BN_CLICKED(IDC_BTN_DONATE, &COptionsDlg::OnBnClickedBtnDonate)
	ON_EN_CHANGE(IDC_EDIT_TIME_SPAN_LONG_BREAK, &COptionsDlg::OnEnChangeEditTimeSpanLongBreak)
	ON_EN_CHANGE(IDC_EDIT_LONG_BREAK_INTERVAL, &COptionsDlg::OnEnChangeEditLongBreakInterval)
	ON_BN_CLICKED(IDC_CHECK_USE_LONG_BREAK, &COptionsDlg::OnBnClickedCheckUseLongBreak)
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

	auto time_span_long_break = cfg.break_time_span_long / 60;
	if (time_span_long_break < 1) time_span_long_break = 1;

	// init spin controls
	m_ctrlSpinTimeSpanWork.SetRange(1, 240);
	m_ctrlSpinTimeSpanWork.SetPos(time_span_work);

	m_ctrlSpinTimeSpanShortBreak.SetRange(1, 240);
	m_ctrlSpinTimeSpanShortBreak.SetPos(time_span_short_break);

	m_ctrlSpinTimeSpanLongBreak.SetRange(1, 240);
	m_ctrlSpinTimeSpanLongBreak.SetPos(time_span_long_break);

	m_ctrlSpinLongBreakInterval.SetRange(1, 100);
	m_ctrlSpinLongBreakInterval.SetPos(cfg.long_break_interval);

	m_ctrlSpinNumLoops.SetRange(0, 100);
	m_ctrlSpinNumLoops.SetPos(cfg.max_loops);

	// init checkboxs
	m_boolUseLongBreak = cfg.use_long_break ? TRUE : FALSE;
	m_boolAutoStart = cfg.auto_start ? TRUE : FALSE;
	m_boolShowSeconds = cfg.show_time_seconds ? TRUE : FALSE;
	m_boolAutoLoop = cfg.auto_loop ? TRUE : FALSE;
	m_boolPlaySound = cfg.play_sound ? TRUE : FALSE;

	// enable/disable controls of loops
	EnableControlsAboutLoops(m_boolAutoLoop);

	// enable/disable controls of sound
	EnableControlsAboutSound(m_boolPlaySound);

	// enable/disable controls of long break
	EnableControlsAboutLongBreak(m_boolUseLongBreak);

	// init sound list combobox
	m_ctrlSoundList.AddString(L"Sound-1");
	m_ctrlSoundList.AddString(L"Sound-2");
	m_ctrlSoundList.AddString(L"Sound-3");
	m_ctrlSoundList.SetCurSel(cfg.sound_id);

	// init radio buttons of the action when user double clicked taskbar window
	m_intRadioDoubleClickAction = cfg.taskbar_dc_action;

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

void COptionsDlg::EnableControlsAboutLongBreak(BOOL enable /* = TRUE */)
{
	GetDlgItem(IDC_EDIT_TIME_SPAN_LONG_BREAK)->EnableWindow(enable);
	GetDlgItem(IDC_SPIN_TIME_SPAN_LONG_BREAK)->EnableWindow(enable);

	GetDlgItem(IDC_EDIT_LONG_BREAK_INTERVAL)->EnableWindow(enable);
	GetDlgItem(IDC_SPIN_LONG_BREAK_INTERVAL)->EnableWindow(enable);
}


void COptionsDlg::OnOK()
{
	auto &data_manager = CDataManager::Instance();

	auto &cfg = CDataManager::Instance().GetConfig();

	// inform user thar time settings will effect in next state if timer is still running
	if (data_manager.GetProgramState() != EProgramState::PS_STOPPED)
	{
		bool time_changed = (cfg.working_time_span != m_ctrlSpinTimeSpanWork.GetPos() * 60) ||
			(cfg.break_time_span != m_ctrlSpinTimeSpanShortBreak.GetPos() * 60);

		if (time_changed)
			MessageBox(data_manager.StringRes(IDS_OPT_DLG_SAVING_WARN),
				data_manager.StringRes(IDS_OPT_DLG_TITLE), MB_OK | MB_ICONWARNING);
	}

	UpdateData(TRUE);

	cfg.working_time_span = m_ctrlSpinTimeSpanWork.GetPos() * 60;
	cfg.break_time_span = m_ctrlSpinTimeSpanShortBreak.GetPos() * 60;

	cfg.break_time_span_long = m_ctrlSpinTimeSpanLongBreak.GetPos() * 60;
	cfg.long_break_interval = m_ctrlSpinLongBreakInterval.GetPos();
	cfg.use_long_break = m_boolUseLongBreak == TRUE;

	cfg.auto_start = m_boolAutoStart == TRUE;

	cfg.auto_loop = m_boolAutoLoop == TRUE;
	cfg.max_loops = m_ctrlSpinNumLoops.GetPos();

	cfg.play_sound = m_boolPlaySound == TRUE;
	cfg.sound_id = m_ctrlSoundList.GetCurSel();

	cfg.show_time_seconds = m_boolShowSeconds == TRUE;

	cfg.taskbar_dc_action = m_intRadioDoubleClickAction;

	CDataManager::Instance().SaveConfig();

	CDialogEx::OnOK();
}


void COptionsDlg::OnBnClickedCheckUseLongBreak()
{
	UpdateData(TRUE);

	EnableControlsAboutLongBreak(m_boolUseLongBreak);
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


void COptionsDlg::OnEnChangeEditTimeSpanLongBreak()
{
	VerifyNumberEditValue(m_ctrlEditTimeSpanLongBreak, m_ctrlSpinTimeSpanLongBreak);
}


void COptionsDlg::OnEnChangeEditLongBreakInterval()
{
	VerifyNumberEditValue(m_ctrlEditLongBreakInterval, m_ctrlSpinLongBreakInterval);
}


void COptionsDlg::OnEnChangeEditNumLoops()
{
	VerifyNumberEditValue(m_ctrlEditNumLoops, m_ctrlSpinNumLoops);
}


void COptionsDlg::OnBnClickedBtnDonate()
{
	CDialogEx dlg(IDD_DLG_DONATE, this);
	dlg.DoModal();
}
