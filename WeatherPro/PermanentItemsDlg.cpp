// PermanentItemsDlg.cpp: 实现文件
//

#include "pch.h"
#include "WeatherPro.h"
#include "afxdialogex.h"
#include "PermanentItemsDlg.h"

#include "DataManager.h"
#include "PermanentItem.h"

#include <unordered_map>


// CPermanentItemsDlg 对话框

IMPLEMENT_DYNAMIC(CPermanentItemsDlg, CDialogEx)

CPermanentItemsDlg::CPermanentItemsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PERMANENT_ITEMS_DLG, pParent)
	, m_rt_humidity(FALSE)
	, m_rt_wind(FALSE)
	, m_rt_pm2p5(FALSE)
	, m_rt_pm10(FALSE)
	, m_rt_aqi(FALSE)
{

}

CPermanentItemsDlg::~CPermanentItemsDlg()
{
}

void CPermanentItemsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_PI_HUMIDITY, m_rt_humidity);
	DDX_Check(pDX, IDC_CHECK_PI_WIND, m_rt_wind);
	DDX_Check(pDX, IDC_CHECK_PI_PM2P5, m_rt_pm2p5);
	DDX_Check(pDX, IDC_CHECK_PI_PM10, m_rt_pm10);
	DDX_Check(pDX, IDC_CHECK_PI_AIR_QUALITY_INDEX, m_rt_aqi);
}


BEGIN_MESSAGE_MAP(CPermanentItemsDlg, CDialogEx)
END_MESSAGE_MAP()


// CPermanentItemsDlg 消息处理程序

BOOL CPermanentItemsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	auto pi_code = CDataManager::Instance().GetConfig().m_permanent_item;
	
	std::unordered_map<PermanmentItemType, BOOL*> tp_pair{
		{PermanmentItemType::RT_Humidity, &m_rt_humidity},
		{PermanmentItemType::RT_Wind, &m_rt_wind},
		{PermanmentItemType::RT_AQI, &m_rt_aqi},
		{PermanmentItemType::RT_PM2P5, &m_rt_pm2p5},
		{PermanmentItemType::RT_PM10, &m_rt_pm10},
	};

	for (const auto &[t, p] : tp_pair) {
		if (static_cast<int>(t) & pi_code) {
			*p = TRUE;
		}
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CPermanentItemsDlg::OnOK()
{
	UpdateData(TRUE);

	std::unordered_map<PermanmentItemType, BOOL*> tp_pair{
		{PermanmentItemType::RT_Humidity, &m_rt_humidity},
		{PermanmentItemType::RT_Wind, &m_rt_wind},
		{PermanmentItemType::RT_AQI, &m_rt_aqi},
		{PermanmentItemType::RT_PM2P5, &m_rt_pm2p5},
		{PermanmentItemType::RT_PM10, &m_rt_pm10},
	};

	int target_pi_code{ 0 };

	for (const auto &[t, p] : tp_pair) {
		if (*p == TRUE) {
			target_pi_code |= static_cast<int>(t);
		}
	}

	auto &cfg = CDataManager::InstanceRef().GetConfig();
	if (target_pi_code != cfg.m_permanent_item) {
		cfg.m_permanent_item = target_pi_code;

		CDataManager::Instance().SaveConfigs();
	}

	CDialogEx::OnOK();
}
