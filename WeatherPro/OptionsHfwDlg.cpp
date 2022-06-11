// OptionsHfwDlg.cpp: 实现文件
//

#include "pch.h"
#include "WeatherPro.h"
#include "afxdialogex.h"
#include "OptionsHfwDlg.h"

#include "DataManager.h"


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
}


BEGIN_MESSAGE_MAP(OptionsHfwDlg, CDialogEx)
END_MESSAGE_MAP()


// OptionsHfwDlg 消息处理程序


BOOL OptionsHfwDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_api != nullptr)
	{
		const auto &config = m_api->config;

		m_key = config.AppKey.c_str();
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
