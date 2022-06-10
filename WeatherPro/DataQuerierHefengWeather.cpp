#include "pch.h"
#include "DataQuerier.h"
#include "Common.h"
#include <afxinet.h>
#include <utilities/yyjson/yyjson.h>
#include <unordered_map>

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

    std::wstring convert_weather_code(const std::wstring &code)
    {
        static const std::unordered_map<std::wstring, std::wstring> dmap{
            {L"100", L"d00"},
            {L"101", L"d01"},
            {L"102", L"d01"},
            {L"103", L"d01"},
            {L"104", L"02"},
            {L"150", L"n00"},
            {L"151", L"n01"},
            {L"152", L"n01"},
            {L"153", L"n01"},
            {L"154", L"02"},
            {L"300", L"d03"},
            {L"301", L"d03"},
            {L"302", L"04"},
            {L"303", L"04"},
            {L"304", L"05"},
            {L"305", L"07"},
            {L"306", L"08"},
            {L"307", L"09"},
            {L"308", L"12"},
            {L"309", L"07"},
            {L"310", L"10"},
            {L"311", L"11"},
            {L"312", L"12"},
            {L"313", L"19"},
            {L"314", L"21"},
            {L"315", L"22"},
            {L"316", L"23"},
            {L"317", L"24"},
            {L"318", L"25"},
            {L"350", L"n03"},
            {L"351", L"n03"},
            {L"399", L"97"},
            {L"400", L"14"},
            {L"401", L"15"},
            {L"402", L"16"},
            {L"403", L"17"},
            {L"404", L"06"},
            {L"405", L"06"},
            {L"406", L"06"},
            {L"407", L"d13"},
            {L"408", L"26"},
            {L"409", L"27"},
            {L"410", L"28"},
            {L"456", L"06"},
            {L"457", L"n13"},
            {L"499", L"98"},
            {L"500", L"18"},
            {L"501", L"18"},
            {L"502", L"53"},
            {L"503", L"30"},
            {L"504", L"29"},
            {L"507", L"20"},
            {L"508", L"31"},
            {L"509", L"32"},
            {L"510", L"49"},
            {L"511", L"54"},
            {L"512", L"55"},
            {L"513", L"56"},
            {L"514", L"57"},
            {L"515", L"58"},
        };

        auto itr = dmap.find(code);
        if (itr != dmap.end())
            return itr->second;
        else
            return L"";
    }
}

ApiHefengWeather::ApiHefengWeather(const std::wstring &key) : _key{key}
{}

const std::wstring &ApiHefengWeather::GetKey() const
{
    return _key;
}

void ApiHefengWeather::SetKey(const std::wstring &key)
{
    _key = key;
}

bool ApiHefengWeather::QueryCityInfo(const std::wstring &query, CityInfoList &city_list)
{
    city_list.clear();

    auto qNameEncoded = CCommon::URLEncode(query);

    CString url;
    url.Format(L"https://geoapi.qweather.com/v2/city/lookup?key=%s&location=%s", _key.c_str(), qNameEncoded.c_str());

    std::wstring content;
    auto succeed = hf::call_internet(url, content);

    if (succeed && !content.empty())
    {
        auto json_utf8 = CCommon::UnicodeToStr(content.c_str(), true);
        yyjson_doc *doc = yyjson_read(json_utf8.c_str(), json_utf8.size(), 0);
        if (doc != nullptr)
        {
            auto *root = yyjson_doc_get_root(doc);

            auto get_str_value = [](yyjson_val *j_val, const char *key) {
                auto *obj = yyjson_obj_get(j_val, key);
                return CCommon::StrToUnicode(yyjson_get_str(obj), true);
            };

            auto get_city_info = [get_str_value](yyjson_val *j_val) {
                SCityInfo city;
                city.CityName = get_str_value(j_val, "name");
                city.CityNO = get_str_value(j_val, "id");
                city.CityAdministrativeOwnership = get_str_value(j_val, "adm2") + L"-" + get_str_value(j_val, "adm1");
                
                return city;
            };

            auto code = get_str_value(root, "code");
            if (std::stoi(code) == 200)
            {
                auto *loc_arr = yyjson_obj_get(root, "location");
                auto num_cities = yyjson_arr_size(loc_arr);

                for (decltype(num_cities) i = 0; i < num_cities; ++i)
                    city_list.push_back(get_city_info(yyjson_arr_get(loc_arr, i)));
            }
            else
            {
                _lastError = L"Error code: " + code;
                succeed = false;
            }
        }
        else
        {
            _lastError = L"Invalid json content.";
            succeed = false;
        }

        yyjson_doc_free(doc);
    }
    else
        _lastError = L"Failed to access the Internet.";

    return succeed;
}

bool ApiHefengWeather::QueryRealTimeWeather(const std::wstring &query, SRealTimeWeather &weather)
{
    CString url;
    url.Format(L"https://devapi.qweather.com/v7/weather/now?key=%s&location=%s", _key.c_str(), query.c_str());

    std::wstring content;
    auto succeed = hf::call_internet(url, content);

    if (succeed && !content.empty())
    {
        auto json_utf8 = CCommon::UnicodeToStr(content.c_str(), true);
        yyjson_doc *doc = yyjson_read(json_utf8.c_str(), json_utf8.size(), 0);
        if (doc != nullptr)
        {
            auto *root = yyjson_doc_get_root(doc);

            auto get_str_value = [](yyjson_val *j_val, const char *key) {
                auto *obj = yyjson_obj_get(j_val, key);
                return CCommon::StrToUnicode(yyjson_get_str(obj), true);
            };

            auto code = get_str_value(root, "code");
            if (std::stoi(code) == 200)
            {
                auto *now_obj = yyjson_obj_get(root, "now");

                weather.Temperature = get_str_value(now_obj, "temp");
                weather.UpdateTime = get_str_value(now_obj, "obsTime").substr(11, 5);
                weather.Weather = get_str_value(now_obj, "text");
                weather.WeatherCode = hf::convert_weather_code(get_str_value(now_obj, "icon"));
                weather.WindDirection = get_str_value(now_obj, "windDir");
                weather.WindStrength = get_str_value(now_obj, "windScale") + L"级";  // todo: 暂时使用中文
                weather.AqiPM25 = L"-";  // todo: 暂时不设置pm2.5
            }
            else
            {
                _lastError = L"Error code: " + code;
                succeed = false;
            }
        }
        else
        {
            _lastError = L"Invalid json contents.";
            succeed = false;
        }

        yyjson_doc_free(doc);
    }
    else
        _lastError = L"Failed to access the Internet.";

    return succeed;
}

bool ApiHefengWeather::QueryWeatherAlerts(const std::wstring &query, WeatherAlertList &alerts)
{
    alerts.clear();

    CString url;
    url.Format(L"https://devapi.qweather.com/v7/warning/now?key=%s&location=%s", _key.c_str(), query.c_str());

    std::wstring content;
    auto succeed = hf::call_internet(url, content);

    if (succeed && !content.empty())
    {
        auto json_utf8 = CCommon::UnicodeToStr(content.c_str(), true);
        yyjson_doc *doc = yyjson_read(json_utf8.c_str(), json_utf8.size(), 0);

        if (doc != nullptr)
        {
            auto *root = yyjson_doc_get_root(doc);

            auto get_str_value = [](yyjson_val *j_val, const char *key) {
                auto *obj = yyjson_obj_get(j_val, key);
                return CCommon::StrToUnicode(yyjson_get_str(obj), true);
            };

            auto get_alert_info = [get_str_value](yyjson_val *j_val) {
                SWeatherAlert alert;
                alert.Type = get_str_value(j_val, "typeName");
                alert.Level = get_str_value(j_val, "severity");
                alert.BriefMessage = get_str_value(j_val, "title");
                alert.DetailedMessage = get_str_value(j_val, "text");
                alert.PublishTime = get_str_value(j_val, "pubTime").substr(11, 5);
                return alert;
            };

            auto code = get_str_value(root, "code");
            if (std::stoi(code) == 200)
            {
                auto *alert_arr = yyjson_obj_get(root, "warning");
                auto num_alerts = yyjson_arr_size(alert_arr);

                for (decltype(num_alerts) i = 0; i < num_alerts; ++i)
                    alerts.push_back(get_alert_info(yyjson_arr_get(alert_arr, i)));
            }
            else
            {
                _lastError = L"Error code: " + code;
                succeed = false;
            }
        }
        else
        {
            _lastError = L"Invalid json contents.";
            succeed = false;
        }

        yyjson_doc_free(doc);
    }
    else
        _lastError = L"Failed to access the Internet.";

    return succeed;
}

bool ApiHefengWeather::QueryForecastWeather(const std::wstring &query, SWeatherInfo &weather_td, SWeatherInfo &weather_tm)
{
    CString url;
    url.Format(L"https://devapi.qweather.com/v7/weather/3d?key=%s&location=%s", _key.c_str(), query.c_str());

    std::wstring content;
    auto succeed = hf::call_internet(url, content);

    if (succeed && !content.empty())
    {
        auto json_utf8 = CCommon::UnicodeToStr(content.c_str(), true);
        yyjson_doc *doc = yyjson_read(json_utf8.c_str(), json_utf8.size(), 0);

        if (doc != nullptr)
        {
            auto *root = yyjson_doc_get_root(doc);

            auto get_str_value = [](yyjson_val *j_val, const char *key) {
                auto *obj = yyjson_obj_get(j_val, key);
                return CCommon::StrToUnicode(yyjson_get_str(obj), true);
            };

            auto get_daily_info = [get_str_value](yyjson_val *j_val, SWeatherInfo &wi) {
                wi.TemperatureDay = get_str_value(j_val, "tempMax");
                wi.TemperatureNight = get_str_value(j_val, "tempMin");
                wi.WeatherDay = get_str_value(j_val, "textDay");
                wi.WeatherNight = get_str_value(j_val, "textNight");
                wi.WeatherCodeDay = hf::convert_weather_code(get_str_value(j_val, "iconDay"));
                wi.WeatherCodeNight = hf::convert_weather_code(get_str_value(j_val, "iconNight"));
            };

            auto code = get_str_value(root, "code");
            if (std::stoi(code) == 200)
            {
                auto *daily_arr = yyjson_obj_get(root, "daily");

                get_daily_info(yyjson_arr_get(daily_arr, 0), weather_td);
                get_daily_info(yyjson_arr_get(daily_arr, 1), weather_tm);
            }
            else
            {
                _lastError = L"Error code: " + code;
                succeed = false;
            }
        }
        else
        {
            _lastError = L"Invalid json contents.";
            succeed = false;
        }

        yyjson_doc_free(doc);
    }
    else
        _lastError = L"Failed to access the Internet.";

    return succeed;
}

std::wstring ApiHefengWeather::GetLastError()
{
    return _lastError;
}
