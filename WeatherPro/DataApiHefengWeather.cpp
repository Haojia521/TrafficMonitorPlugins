#include "pch.h"
#include "DataApiHefengWeather.h"

#include "Common.h"
#include <afxinet.h>
#include <utilities/yyjson/yyjson.h>
#include <unordered_map>
#include <functional>
#include <sstream>

#include "DataManager.h"

#define CHECK_KEY if (config.AppKey.empty()) {\
    _lastError = L"no application key";\
    return false;\
}

namespace hf
{
    bool call_internet(const CString &url, std::wstring &content)
    {
        content.clear();

        bool succeed = false;

        CInternetSession *session{ nullptr };
        CHttpFile *httpFile{ nullptr };

        char *http_content_buffer{ nullptr };
        size_t http_content_len{ 0 };

        unsigned char *contents_buffer = new unsigned char[1024 * 64]{ 0 };  // 假设解压后内容长度不会超过64k字节
        unsigned long contents_len{ 0 };

        try
        {
            session = new CInternetSession();
            httpFile = (CHttpFile *)session->OpenURL(url);

            DWORD dwStatusCode;
            httpFile->QueryInfoStatusCode(dwStatusCode);

            if (dwStatusCode == HTTP_STATUS_OK)
            {
                http_content_len = httpFile->Seek(0, CFile::end);
                http_content_buffer = new char[http_content_len + 1]{ 0 };

                httpFile->Seek(0, CFile::begin);
                httpFile->Read(http_content_buffer, static_cast<UINT>(http_content_len + 1));
            }

            httpFile->Close();
            session->Close();
        }
        catch (CInternetException *e)
        {
            if (httpFile != nullptr) httpFile->Close();
            if (session != nullptr) session->Close();

            succeed = false;
            e->Delete();
        }


        // decompress
        if (CCommon::GZipDecompress((Byte*)http_content_buffer, (uLong)http_content_len,
            contents_buffer, &contents_len) == 0)
        {
            content = CCommon::StrToUnicode((char*)contents_buffer, true);
            succeed = true;
        }

        delete[] http_content_buffer;
        delete[] contents_buffer;
        delete httpFile;
        delete session;

        return succeed;
    }

    //std::wstring convert_weather_code(const std::wstring &code)
    //{
    //    static const std::unordered_map<std::wstring, std::wstring> dmap{
    //        {L"100", L"d00"},
    //        {L"101", L"d01"},
    //        {L"102", L"d01"},
    //        {L"103", L"d01"},
    //        {L"104", L"02"},
    //        {L"150", L"n00"},
    //        {L"151", L"n01"},
    //        {L"152", L"n01"},
    //        {L"153", L"n01"},
    //        {L"154", L"02"},
    //        {L"300", L"d03"},
    //        {L"301", L"d03"},
    //        {L"302", L"04"},
    //        {L"303", L"04"},
    //        {L"304", L"05"},
    //        {L"305", L"07"},
    //        {L"306", L"08"},
    //        {L"307", L"09"},
    //        {L"308", L"12"},
    //        {L"309", L"07"},
    //        {L"310", L"10"},
    //        {L"311", L"11"},
    //        {L"312", L"12"},
    //        {L"313", L"19"},
    //        {L"314", L"21"},
    //        {L"315", L"22"},
    //        {L"316", L"23"},
    //        {L"317", L"24"},
    //        {L"318", L"25"},
    //        {L"350", L"n03"},
    //        {L"351", L"n03"},
    //        {L"399", L"97"},
    //        {L"400", L"14"},
    //        {L"401", L"15"},
    //        {L"402", L"16"},
    //        {L"403", L"17"},
    //        {L"404", L"06"},
    //        {L"405", L"06"},
    //        {L"406", L"06"},
    //        {L"407", L"d13"},
    //        {L"408", L"26"},
    //        {L"409", L"27"},
    //        {L"410", L"28"},
    //        {L"456", L"06"},
    //        {L"457", L"n13"},
    //        {L"499", L"98"},
    //        {L"500", L"18"},
    //        {L"501", L"18"},
    //        {L"502", L"53"},
    //        {L"503", L"30"},
    //        {L"504", L"29"},
    //        {L"507", L"20"},
    //        {L"508", L"31"},
    //        {L"509", L"32"},
    //        {L"510", L"49"},
    //        {L"511", L"54"},
    //        {L"512", L"55"},
    //        {L"513", L"56"},
    //        {L"514", L"57"},
    //        {L"515", L"58"},
    //    };

    //    auto itr = dmap.find(code);
    //    if (itr != dmap.end())
    //        return itr->second;
    //    else
    //        return L"";
    //}

    std::wstring get_json_str_value(yyjson_val *j_val, const char *key)
    {
        auto *obj = yyjson_obj_get(j_val, key);
        if (obj == nullptr) return L"";
        else return CCommon::StrToUnicode(yyjson_get_str(obj), true);
    }

    std::wstring query_func_frame(const CString &url, std::function<void(yyjson_val*)> func)
    {
        std::wstring error;

        std::wstring content;
        auto succeed = call_internet(url, content);

        if (succeed && !content.empty())
        {
            auto json_utf8 = CCommon::UnicodeToStr(content.c_str(), true);
            yyjson_doc *doc = yyjson_read(json_utf8.c_str(), json_utf8.size(), 0);
            if (doc != nullptr)
            {
                auto *root = yyjson_doc_get_root(doc);

                auto code = get_json_str_value(root, "code");
                if (std::stoi(code) == 200)
                {
                    func(root);
                }
                else
                    error = L"Error code: " + code;
            }
            else
                error = L"Invalid json contents.";

            yyjson_doc_free(doc);
        }
        else
            error = L"Failed to access the Internet.";

        return error;
    }
}

bool DataApiHefengWeather::QueryCity(const std::wstring &query, CityInfoList &info)
{
    CHECK_KEY;

    info.clear();

    auto queryEncoded = CCommon::URLEncode(query);

    CString url;
    url.Format(L"https://geoapi.qweather.com/v2/city/lookup?key=%s&location=%s", config.AppKey.c_str(), queryEncoded.c_str());

    CityInfoList cities;
    auto func = [&cities](yyjson_val *j_val) {
        auto *loc_arr = yyjson_obj_get(j_val, "location");
        auto num_cities = yyjson_arr_size(loc_arr);

        auto get_city_info = [](yyjson_val *j_val) {
            SCityInfo city;
            city.CityName = hf::get_json_str_value(j_val, "name");
            city.CityNO = hf::get_json_str_value(j_val, "id");
            city.CityAdministrativeOwnership = hf::get_json_str_value(j_val, "adm2") + L"-" + hf::get_json_str_value(j_val, "adm1");

            return city;
        };

        for (decltype(num_cities) i = 0; i < num_cities; ++i)
            cities.push_back(get_city_info(yyjson_arr_get(loc_arr, i)));
    };

    auto err = hf::query_func_frame(url, func);
    if (err.empty())
        info = cities;
    else
        _lastError = err;

    return err.empty();
}

std::wstring DataApiHefengWeather::GetWeatherInfoSummary()
{
    const auto &app_config = CDataManager::Instance().GetConfig();
    const auto &city_info = CDataManager::Instance().GetCurrentCityInfo();

    std::wostringstream oss_err;

    if (config.ShowAirQuality && _airQualityDataOutdated)
    {
        if (!QueryRealtimeAirQuality(city_info.CityNO))
            oss_err << _lastError;
    }
    if (config.ShowWeatherAlert && _alertsDataOutdated)
    {
        if (!QueryWeatherAlerts(city_info.CityNO))
            oss_err << std::endl << _lastError;
    }

    _lastError = oss_err.str();

    std::wostringstream oss;

    // format realtime weather information
    oss << GetWeatherText(EWeatherInfoType::WEATHER_REALTIME) << L" "
        << GetTemprature(EWeatherInfoType::WEATHER_REALTIME) << L" "
        << L"(" << _realtimeWeather.UpdateTime << L")";
    if (!app_config.m_show_brief_rt_weather_info)
    {
        if (config.ShowRealtimeWind)
        {
            oss << L" " << _realtimeWeather.WindDirection;
            if (config.ShowRealtimeWindScale)
                oss << _realtimeWeather.WindScale << L"级";
            else
                oss << _realtimeWeather.WindSpeed << L"km/h";
        }

        if (config.ShowRealtimeHumidity)
            oss << L" 相对湿度 " << _realtimeWeather.Humidity << L"%";


        // format realtime air quality information
        if (config.ShowAirQuality)
        {
            oss << std::endl << L"空气质量: " << _realtimeAirQuality.Category;

            if (config.ShowAirQualityAQI)
                oss << L" AQI: " << _realtimeAirQuality.AQI;

            if (config.ShowAirQualityPM2p5)
                oss << L" PM2.5: " << _realtimeAirQuality.PM2p5;

            if (config.ShowAirQualityPM10)
                oss << L" PM10: " << _realtimeAirQuality.PM10;
        }
    }
    oss << std::endl;

    // format weather alerts information
    if (app_config.m_show_weather_alerts && config.ShowWeatherAlert && !_weatherAlerts.empty())
    {
        if (app_config.m_show_brief_weather_alert_info)
            for (const auto &alert : _weatherAlerts)
                oss << L"[!] " << alert.Title << L"(" << alert.PublishTime << L")" << std::endl;
        else
            for (const auto &alert : _weatherAlerts)
                oss << L"[!] " << alert.Text << std::endl;
    }

    // format forcast weather information
    // - today
    oss << L"今天: " << GetWeatherText(EWeatherInfoType::WEATHER_TODAY)
        << L" " << GetTemprature(EWeatherInfoType::WEATHER_TODAY);
    if (config.ShowForecastUVIdex)
        oss << L" 紫外线强度: " << _forcastWeatherTD.UVIndex;
    if (config.showForecastHumidity)
        oss << L" 相对湿度: " << _forcastWeatherTD.Humidity << L"%";
    oss << std::endl;
    // - tomorrow
    oss << L"明天: " << GetWeatherText(EWeatherInfoType::WEATHER_TOMMROW)
        << L" " << GetTemprature(EWeatherInfoType::WEATHER_TOMMROW);
    if (config.ShowForecastUVIdex)
        oss << L" 紫外线强度: " << _forcastWeatherTM.UVIndex;
    if (config.showForecastHumidity)
        oss << L" 相对湿度: " << _forcastWeatherTM.Humidity << L"%";
    oss << std::endl;
    // - day after tomorrow
    oss << L"后天: " << GetWeatherText(EWeatherInfoType::WEATHER_DAY_AFTER_TOMMROW)
        << L" " << GetTemprature(EWeatherInfoType::WEATHER_DAY_AFTER_TOMMROW);
    if (config.ShowForecastUVIdex)
        oss << L" 紫外线强度: " << _forcastWeatherDATM.UVIndex;
    if (config.showForecastHumidity)
        oss << L" 相对湿度: " << _forcastWeatherDATM.Humidity << L"%";
    oss << std::endl;

    // end

    return oss.str();
}

std::wstring DataApiHefengWeather::GetTemprature(EWeatherInfoType type)
{
    std::wostringstream oss;

    switch (type)
    {
    case EWeatherInfoType::WEATHER_REALTIME:
        if (config.ShowRealtimeTemperatureFeelsLike)
            oss << _realtimeWeather.TemperatureFeelsLike << L"℃";
        else
            oss << _realtimeWeather.Temperature << L"℃";
        break;

    case EWeatherInfoType::WEATHER_TODAY:
        oss << _forcastWeatherTD.TemperatureMin << L"℃~" << _forcastWeatherTD.TemperatureMax << L"℃";
        break;

    case EWeatherInfoType::WEATHER_TOMMROW:
        oss << _forcastWeatherTM.TemperatureMin << L"℃~" << _forcastWeatherTM.TemperatureMax << L"℃";
        break;

    case EWeatherInfoType::WEATHER_DAY_AFTER_TOMMROW:
        oss << _forcastWeatherDATM.TemperatureMin << L"℃~" << _forcastWeatherDATM.TemperatureMax << L"℃";
        break;

    default:
        break;
    }

    return oss.str();
}

std::wstring DataApiHefengWeather::GetWeatherText(EWeatherInfoType type)
{
    std::wostringstream oss;

    switch (type)
    {
    default:
    case EWeatherInfoType::WEATHER_REALTIME:
        oss << _realtimeWeather.WeatherText;
        break;

    case EWeatherInfoType::WEATHER_TODAY:
        if (_forcastWeatherTD.WeatherDay == _forcastWeatherTD.WeatherNight)
            oss << _forcastWeatherTD.WeatherDay;
        else
            oss << _forcastWeatherTD.WeatherDay << L"转" << _forcastWeatherTD.WeatherNight;
        break;

    case EWeatherInfoType::WEATHER_TOMMROW:
        if (_forcastWeatherTM.WeatherDay == _forcastWeatherTM.WeatherNight)
            oss << _forcastWeatherTM.WeatherDay;
        else
            oss << _forcastWeatherTM.WeatherDay << L"转" << _forcastWeatherTM.WeatherNight;
        break;
    case EWeatherInfoType::WEATHER_DAY_AFTER_TOMMROW:
        if (_forcastWeatherDATM.WeatherDay == _forcastWeatherDATM.WeatherNight)
            oss << _forcastWeatherDATM.WeatherDay;
        else
            oss << _forcastWeatherDATM.WeatherDay << L"转" << _forcastWeatherDATM.WeatherNight;
        break;
    }

    return oss.str();
}

std::wstring DataApiHefengWeather::GetWeatherCode(EWeatherInfoType type)
{
    switch (type)
    {
    default:
    case EWeatherInfoType::WEATHER_REALTIME:
        return _realtimeWeather.WeatherCode;
    case EWeatherInfoType::WEATHER_TODAY:
        return _forcastWeatherTD.WeatherCodeDay;
    case EWeatherInfoType::WEATHER_TOMMROW:
        return _forcastWeatherTM.WeatherCodeDay;
    case EWeatherInfoType::WEATHER_DAY_AFTER_TOMMROW:
        return _forcastWeatherDATM.WeatherCodeDay;
    }
}

bool DataApiHefengWeather::UpdateWeather()
{
    CHECK_KEY;

    _airQualityDataOutdated = true;
    _alertsDataOutdated = true;

    std::wostringstream oss;

    const auto &currunt_city = CDataManager::Instance().GetCurrentCityInfo();

    if (!QueryRealtimeWeather(currunt_city.CityNO))
        oss << L"[RealtimeWeather]" << _lastError << std::endl;
    if (!QueryForecastWeather(currunt_city.CityNO))
        oss << L"[ForcastWeather]" << _lastError << std::endl;
    if (config.ShowAirQuality && !QueryRealtimeAirQuality(currunt_city.CityNO))
        oss << L"[RealtimeAirQuality]" << _lastError << std::endl;
    if (config.ShowWeatherAlert && !QueryWeatherAlerts(currunt_city.CityNO))
        oss << L"[WeatherAlerts]" << _lastError;

    _lastError = oss.str();

    return _lastError.empty();
}

std::wstring DataApiHefengWeather::GetLastError()
{
    return _lastError;
}

bool DataApiHefengWeather::QueryRealtimeWeather(const std::wstring &query)
{
    CHECK_KEY;

    CString url;
    url.Format(L"https://devapi.qweather.com/v7/weather/now?key=%s&location=%s", config.AppKey.c_str(), query.c_str());

    RealtimeWeather data;
    auto func = [&data](yyjson_val *j_val) {
        auto *now_obj = yyjson_obj_get(j_val, "now");

        data.Temperature = hf::get_json_str_value(now_obj, "temp");
        data.TemperatureFeelsLike = hf::get_json_str_value(now_obj, "feelsLike");
        data.UpdateTime = hf::get_json_str_value(now_obj, "obsTime").substr(11, 5);
        data.WeatherText = hf::get_json_str_value(now_obj, "text");
        //data.WeatherCode = hf::convert_weather_code(hf::get_json_str_value(now_obj, "icon"));
        data.WeatherCode = hf::get_json_str_value(now_obj, "icon");
        data.WindDirection = hf::get_json_str_value(now_obj, "windDir");
        data.WindScale = hf::get_json_str_value(now_obj, "windScale");
        data.WindSpeed = hf::get_json_str_value(now_obj, "windSpeed");
        data.Humidity = hf::get_json_str_value(now_obj, "humidity");
    };

    auto err = hf::query_func_frame(url, func);
    if (err.empty())
        _realtimeWeather = data;
    else
        _lastError = err;

    return err.empty();
}

bool DataApiHefengWeather::QueryRealtimeAirQuality(const std::wstring &query)
{
    CHECK_KEY;

    CString url;
    url.Format(L"https://devapi.qweather.com/v7/air/now?key=%s&location=%s", config.AppKey.c_str(), query.c_str());

    RealtimeAirQuality data;
    auto func = [&data](yyjson_val *j_val) {
        auto *now_obj = yyjson_obj_get(j_val, "now");

        data.PublishTime = hf::get_json_str_value(now_obj, "pubTime").substr(11, 5);
        data.AQI = hf::get_json_str_value(now_obj, "aqi");
        data.Level = hf::get_json_str_value(now_obj, "level");
        data.Category = hf::get_json_str_value(now_obj, "category");
        data.PM2p5 = hf::get_json_str_value(now_obj, "pm2p5");
        data.PM10 = hf::get_json_str_value(now_obj, "pm10");
    };

    auto err = hf::query_func_frame(url, func);
    if (err.empty())
    {
        _realtimeAirQuality = data;
        _airQualityDataOutdated = false;
    }
    else
        _lastError = err;

    return err.empty();
}

bool DataApiHefengWeather::QueryForecastWeather(const std::wstring &query)
{
    CHECK_KEY;

    CString url;
    url.Format(L"https://devapi.qweather.com/v7/weather/3d?key=%s&location=%s", config.AppKey.c_str(), query.c_str());

    ForcastWeather td, tm, datm;
    auto func = [&td, &tm, &datm](yyjson_val *j_val) {
        auto *daily_arr = yyjson_obj_get(j_val, "daily");

        auto get_daily_info = [](yyjson_val *j_val, ForcastWeather &w) {
            w.TemperatureMax = hf::get_json_str_value(j_val, "tempMax");
            w.TemperatureMin = hf::get_json_str_value(j_val, "tempMin");
            w.WeatherDay = hf::get_json_str_value(j_val, "textDay");
            w.WeatherNight = hf::get_json_str_value(j_val, "textNight");
            //w.WeatherCodeDay = hf::convert_weather_code(hf::get_json_str_value(j_val, "iconDay"));
            //w.WeatherCodeNight = hf::convert_weather_code(hf::get_json_str_value(j_val, "iconNight"));
            w.WeatherCodeDay = hf::get_json_str_value(j_val, "iconDay");
            w.WeatherCodeNight = hf::get_json_str_value(j_val, "iconNight");
            w.UVIndex = hf::get_json_str_value(j_val, "uvIndex");
            w.Humidity = hf::get_json_str_value(j_val, "humidity");
        };

        get_daily_info(yyjson_arr_get(daily_arr, 0), td);
        get_daily_info(yyjson_arr_get(daily_arr, 1), tm);
        get_daily_info(yyjson_arr_get(daily_arr, 2), datm);
    };

    auto err = hf::query_func_frame(url, func);
    if (err.empty())
    {
        _forcastWeatherTD = td;
        _forcastWeatherTM = tm;
        _forcastWeatherDATM = datm;
    }
    else
        _lastError = err;

    return err.empty();
}

bool DataApiHefengWeather::QueryWeatherAlerts(const std::wstring &query)
{
    CHECK_KEY;

    CString url;
    url.Format(L"https://devapi.qweather.com/v7/warning/now?key=%s&location=%s", config.AppKey.c_str(), query.c_str());

    WeatherAlertList data;
    auto func = [&data](yyjson_val *j_val) {
        auto *alert_arr = yyjson_obj_get(j_val, "warning");
        auto num_alerts = yyjson_arr_size(alert_arr);

        auto get_alert_info = [](yyjson_val *j_val) {
            WeatherAlert alert;
            alert.Type = hf::get_json_str_value(j_val, "type");
            alert.TypeName = hf::get_json_str_value(j_val, "typeName");
            alert.Level = hf::get_json_str_value(j_val, "level");
            alert.Title = hf::get_json_str_value(j_val, "title");
            alert.Text = hf::get_json_str_value(j_val, "text");
            alert.Severity = hf::get_json_str_value(j_val, "severity");
            alert.SeverityColor = hf::get_json_str_value(j_val, "severityColor");
            alert.PublishTime = hf::get_json_str_value(j_val, "pubTime").substr(0, 16).replace(10, 1, L" ");
            return alert;
        };

        for (decltype(num_alerts) i = 0; i < num_alerts; ++i)
            data.push_back(get_alert_info(yyjson_arr_get(alert_arr, i)));
    };

    auto err = hf::query_func_frame(url, func);
    if (err.empty())
    {
        _weatherAlerts = data;
        _alertsDataOutdated = false;
    }
    else
        _lastError = err;

    return err.empty();
}
