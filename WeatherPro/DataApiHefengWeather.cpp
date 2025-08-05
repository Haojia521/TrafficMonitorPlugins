#include "pch.h"
#include "DataApiHefengWeather.h"

#include "Common.h"
#include <afxinet.h>
#include <utilities/yyjson/yyjson.h>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <format>
#include <memory>
#include <chrono>
#include <jwt-cpp/jwt.h>

#include "DataManager.h"

#define CHECK_APIHOST if (config.ApiHost.empty()) {\
    errors.push_back(L"no API host");\
    return false;\
}

#define CHECK_KEY if (!config.AuthViaJWT && config.AppKey.empty()) {\
    errors.push_back(L"no application key");\
    return false;\
}

#define CHECK_JWT_SUB_ID if (config.AuthViaJWT && config.ProjectID.empty()) {\
    errors.push_back(L"no project ID");\
    return false;\
}

#define CHECK_JWT_CRD_ID if (config.AuthViaJWT && config.CredentialID.empty()) {\
    errors.push_back(L"no JWT credential ID");\
    return false;\
}

#define CHECK_JWT CHECK_JWT_SUB_ID CHECK_JWT_CRD_ID

#define HFW_CHECK CHECK_APIHOST CHECK_KEY CHECK_JWT

namespace hf
{
    static std::wstring json_get_str_value(yyjson_val *j_val, const char *key)
    {
        auto *obj = yyjson_obj_get(j_val, key);
        if (obj == nullptr) return L"";
        else return utils::multi_byte2wide_char(yyjson_get_str(obj));
    }

    static bool json_has_obj(yyjson_val *j_val, const char *key)
    {
        return yyjson_obj_get(j_val, key) != nullptr;
    }

    static std::wstring format_error_v2(yyjson_val *j_err)
    {
        auto status = yyjson_get_int(yyjson_obj_get(j_err, "status"));
        auto title = json_get_str_value(j_err, "title");
        auto detail = json_get_str_value(j_err, "detail");

        return std::format(L"[{}] {} ({})", status, title, detail);
    }

    static bool query_func_frame(const std::string &host, const std::string &path, std::function<void(yyjson_val*)> func, WStringList &errors,
                                 httplib::Headers http_headers = httplib::Headers())
    {
        const auto &dm = CDataManager::Instance();

        bool succeed{ false };
        std::wstring content;
        auto status_code = utils::internet_get(host, path, content, errors, http_headers);

        if (!content.empty()) {
            auto json_utf8 = utils::wide_char2multi_byte(content.c_str());
            std::unique_ptr<yyjson_doc, void(*)(yyjson_doc*)> doc(
                yyjson_read(json_utf8.c_str(), json_utf8.size(), 0),
                [](yyjson_doc *p) { yyjson_doc_free(p); }
            );

            if (doc != nullptr) {
                auto *root = yyjson_doc_get_root(doc.get());

                if (status_code == 200) {
                    // compatible with error code v1
                    auto code = json_get_str_value(root, "code");
                    if (code == L"200") {
                        func(root);
                        succeed = true;
                    } else {
                        errors.emplace_back(std::format(L"{} {}", dm.StringRes(IDS_HFW_ERROR_CODE).GetString(), code));
                    }
                }
                else {
                    // compatible with error code v2
                    if (json_has_obj(root, "error")) {
                        errors.emplace_back(format_error_v2(yyjson_obj_get(root, "error")));
                    } else
                        errors.emplace_back(std::format(L"[{}] unknown error", status_code));
                }
            } else
                errors.emplace_back(dm.StringRes(IDS_HFW_INVALID_JSON));
        } else
            errors.emplace_back(std::format(L"[{}] {}", status_code, dm.StringRes(IDS_HFW_NO_INTERNET).GetString()));

        return succeed;
    }

    static const UpdatingMask um_realtime_weather{ 1ull << 0 };
    static const UpdatingMask um_forecast_weather{ 1ull << 1 };
    static const UpdatingMask um_realtime_aq{ 1ull << 2 };
    static const UpdatingMask um_weather_alerts{ 1ull << 3 };

    static const std::chrono::seconds jwt_time_offset{ 30 };
    static const std::chrono::seconds jwt_time_duration{ 900 };

    std::string generate_jwt(const DataApiHefengWeather::Config &cfg, WStringList &errors)
    {
        static std::chrono::system_clock::time_point timestamp;
        static std::string token_cache;

        auto now = std::chrono::system_clock::now();

        if ((now < timestamp + jwt_time_duration - jwt_time_offset) && !token_cache.empty()) {
            return token_cache;
        }

        std::ifstream ifs(cfg.JwtPrivateKeyFile);

        std::string private_key;
        if (ifs.is_open()) {
            private_key.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

            ifs.close();
        } else {
            errors.push_back(L"[GenJWT] cannot open private key file");
            return "";
        }

        auto project_id = utils::wide_char2multi_byte(cfg.ProjectID.c_str());
        auto credential_id = utils::wide_char2multi_byte(cfg.CredentialID.c_str());

        try {
            auto token = jwt::create()
                .set_issued_at(now - jwt_time_offset)
                .set_expires_at(now + jwt_time_duration)
                .set_subject(project_id)
                .set_header_claim("kid", jwt::claim(credential_id))
                .sign(jwt::algorithm::ed25519("", private_key));

            timestamp = now;
            token_cache = token;
        }
        catch (const std::exception &e) {
            errors.push_back(utils::multi_byte2wide_char(e.what()));

            token_cache.clear();
        }

        return token_cache;
    }
}

bool DataApiHefengWeather::QueryCity(const std::wstring &query, CityInfoList &info, WStringList &errors)
{
    HFW_CHECK;

    info.clear();
    const auto &dm = CDataManager::Instance();

    std::string url_host = std::format("https://{}", utils::wide_char2multi_byte(config.ApiHost.c_str()));
    std::string url_path;
    httplib::Headers http_headers;
    
    if (config.AuthViaJWT) {
        auto jwt = hf::generate_jwt(config, errors);
        if (jwt.empty()) {
            return false;
        }

        http_headers.emplace(std::make_pair("Authorization", std::format("Bearer {}", jwt)));

        url_path = utils::wide_char2multi_byte(
            std::format(L"/geo/v2/city/lookup?location={}&lang={}",
                        query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    } else {
        url_path = utils::wide_char2multi_byte(
            std::format(L"/geo/v2/city/lookup?key={}&location={}&lang={}",
                        config.AppKey, query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    }

    auto func = [&info](yyjson_val *j_val) {
        auto *loc_arr = yyjson_obj_get(j_val, "location");
        auto num_cities = yyjson_arr_size(loc_arr);

        auto get_city_info = [](yyjson_val *j_val) {
            SCityInfo city;
            city.CityName = hf::json_get_str_value(j_val, "name");
            city.CityNO = hf::json_get_str_value(j_val, "id");
            city.CityAdministrativeOwnership = hf::json_get_str_value(j_val, "adm2") + L"-" + hf::json_get_str_value(j_val, "adm1");

            return city;
        };

        for (decltype(num_cities) i = 0; i < num_cities; ++i)
            info.push_back(get_city_info(yyjson_arr_get(loc_arr, i)));
    };

    if (!hf::query_func_frame(url_host, url_path, func, errors, http_headers)) {
        errors.push_back(L"QueryCity failed");
        return false;
    }

    return true;
}

std::wstring DataApiHefengWeather::GetWeatherInfoSummary(WStringList &errors)
{
    const auto &app_config = CDataManager::Instance().GetConfig();
    const auto &city_info = CDataManager::Instance().GetCurrentCityInfo();

    if (config.ShowAirQuality && _airQualityDataOutdated)
        QueryRealtimeAirQuality(city_info.CityNO, errors);

    if (config.ShowWeatherAlert && _alertsDataOutdated)
        QueryWeatherAlerts(city_info.CityNO, errors);

    const auto &dm = CDataManager::Instance();
    CString tmp_str;

    std::wostringstream oss;

    // format realtime weather information
    oss << GetWeatherText(EWeatherInfoType::WEATHER_REALTIME) << L" "
        << GetTemprature(EWeatherInfoType::WEATHER_REALTIME) << L" "
        << L"(" << _realtimeWeather.UpdateTime << L")";
    if (!app_config.m_show_brief_rt_weather_info)
    {
        if (config.ShowRealtimeWind)
        {
            tmp_str.Format(dm.StringRes(IDS_HFW_FMT_WIND_DIRECTION), _realtimeWeather.WindDirection.c_str());
            oss << L" " << tmp_str.GetString();
            if (config.ShowRealtimeWindScale)
            {
                tmp_str.Format(dm.StringRes(IDS_HFW_FMT_WIND_SCALE), _realtimeWeather.WindScale.c_str());
                oss << tmp_str.GetString();
            }
            else
                oss << _realtimeWeather.WindSpeed << L"km/h";
        }

        if (config.ShowRealtimeHumidity)
            oss << L" " << dm.StringRes(IDS_HFW_FMT_HUMIDITY).GetString() << _realtimeWeather.Humidity << L"%";

        // format realtime air quality information
        if (config.ShowAirQuality)
        {
            oss << std::endl << dm.StringRes(IDS_HFW_FMT_AIR_QUALITY).GetString() << _realtimeAirQuality.Category;

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
    auto forcast_formatter = [&dm, &oss, this](EWeatherInfoType weather_type, ForcastWeather &weather, UINT date_res_id) {
        oss << dm.StringRes(date_res_id).GetString() << L": " << GetWeatherText(weather_type)
            << L" " << GetTemprature(weather_type);
        if (config.ShowForecastUVIdex)
            oss << L" " << dm.StringRes(IDS_HFW_FMT_UVI).GetString() << weather.UVIndex;
        if (config.showForecastHumidity)
            oss << L" " << dm.StringRes(IDS_HFW_FMT_HUMIDITY).GetString() << weather.Humidity << L"%";
        oss << std::endl;
        };

    forcast_formatter(EWeatherInfoType::WEATHER_TODAY, _forcastWeatherTD, IDS_TODAY);
    forcast_formatter(EWeatherInfoType::WEATHER_TOMMROW, _forcastWeatherTM, IDS_TOMORROW);
    forcast_formatter(EWeatherInfoType::WEATHER_DAY_AFTER_TOMMROW, _forcastWeatherDATM, IDS_DAY_AFTER_TOMORROW);

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
            oss << _forcastWeatherTD.WeatherDay << L"~" << _forcastWeatherTD.WeatherNight;
        break;

    case EWeatherInfoType::WEATHER_TOMMROW:
        if (_forcastWeatherTM.WeatherDay == _forcastWeatherTM.WeatherNight)
            oss << _forcastWeatherTM.WeatherDay;
        else
            oss << _forcastWeatherTM.WeatherDay << L"~" << _forcastWeatherTM.WeatherNight;
        break;
    case EWeatherInfoType::WEATHER_DAY_AFTER_TOMMROW:
        if (_forcastWeatherDATM.WeatherDay == _forcastWeatherDATM.WeatherNight)
            oss << _forcastWeatherDATM.WeatherDay;
        else
            oss << _forcastWeatherDATM.WeatherDay << L"~" << _forcastWeatherDATM.WeatherNight;
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

std::wstring DataApiHefengWeather::GetHumidity(EWeatherInfoType type)
{
    if (type == EWeatherInfoType::WEATHER_REALTIME) {
        return std::format(L"{}%", _realtimeWeather.Humidity);
    }

    return L"-";
}

std::wstring DataApiHefengWeather::GetWind(EWeatherInfoType type)
{
    if (type == EWeatherInfoType::WEATHER_REALTIME) {
        const auto &dm = CDataManager::Instance();

        std::wstring scale_or_speead;
        CString tmp;
        if (config.ShowRealtimeWindScale) {
            tmp.Format(dm.StringRes(IDS_HFW_FMT_WIND_SCALE), _realtimeWeather.WindScale.c_str());
            scale_or_speead = tmp.GetString();
        } else {
            scale_or_speead = std::format(L"{}km/h", _realtimeWeather.WindSpeed);
        }

        return std::format(L"{} {}", _realtimeWeather.WindDirection, scale_or_speead);
    }

    return L"-";
}

std::wstring DataApiHefengWeather::GetAQI(EWeatherInfoType type)
{
    if (type == EWeatherInfoType::WEATHER_REALTIME) {
        return std::format(L"{}({})", _realtimeAirQuality.Category, _realtimeAirQuality.AQI);
    }

    return L"-";
}

std::wstring DataApiHefengWeather::GetPM2p5(EWeatherInfoType type)
{
    if (type == EWeatherInfoType::WEATHER_REALTIME) {
        return _realtimeAirQuality.PM2p5;
    }

    return L"-";
}

std::wstring DataApiHefengWeather::GetPM10(EWeatherInfoType type)
{
    if (type == EWeatherInfoType::WEATHER_REALTIME) {
        return _realtimeAirQuality.PM10;
    }

    return L"-";
}

bool DataApiHefengWeather::UpdateWeather(WStringList &errors, UpdatingMask &mask)
{
    CHECK_KEY;

    _airQualityDataOutdated = true;
    _alertsDataOutdated = true;

    const auto &currunt_city = CDataManager::Instance().GetCurrentCityInfo();

    UpdatingMask target_mask = mask;
    auto do_query =
        [&](DataApiHefengWeather *o,
            std::function<bool(DataApiHefengWeather*, const std::wstring&, WStringList&)> q_func,
            const UpdatingMask &um) {
                target_mask |= um;
                if ((mask & um).none() && q_func(o, currunt_city.CityNO, errors))
                    mask |= um;
        };

    do_query(this, &DataApiHefengWeather::QueryRealtimeWeather, hf::um_realtime_weather);
    do_query(this, &DataApiHefengWeather::QueryForecastWeather, hf::um_forecast_weather);
    if (config.ShowAirQuality) do_query(this, &DataApiHefengWeather::QueryRealtimeAirQuality, hf::um_realtime_aq);
    if (config.ShowWeatherAlert) do_query(this, &DataApiHefengWeather::QueryWeatherAlerts, hf::um_weather_alerts);

    return target_mask == mask;
}

bool DataApiHefengWeather::QueryRealtimeWeather(const std::wstring &query, WStringList &errors)
{
    HFW_CHECK;

    _realtimeWeather = RealtimeWeather();
    const auto &dm = CDataManager::Instance();

    std::string url_host = std::format("https://{}", utils::wide_char2multi_byte(config.ApiHost.c_str()));
    std::string url_path;
    httplib::Headers http_headers;

    if (config.AuthViaJWT) {
        auto jwt = hf::generate_jwt(config, errors);
        if (jwt.empty()) {
            return false;
        }

        http_headers.emplace(std::make_pair("Authorization", std::format("Bearer {}", jwt)));

        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/weather/now?location={}&lang={}",
                        query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    } else {
        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/weather/now?key={}&location={}&lang={}",
                        config.AppKey, query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    }

    RealtimeWeather data;
    auto func = [&data](yyjson_val *j_val) {
        auto *now_obj = yyjson_obj_get(j_val, "now");

        data.Temperature = hf::json_get_str_value(now_obj, "temp");
        data.TemperatureFeelsLike = hf::json_get_str_value(now_obj, "feelsLike");
        data.UpdateTime = hf::json_get_str_value(now_obj, "obsTime").substr(11, 5);
        data.WeatherText = hf::json_get_str_value(now_obj, "text");
        data.WeatherCode = hf::json_get_str_value(now_obj, "icon");
        data.WindDirection = hf::json_get_str_value(now_obj, "windDir");
        data.WindScale = hf::json_get_str_value(now_obj, "windScale");
        data.WindSpeed = hf::json_get_str_value(now_obj, "windSpeed");
        data.Humidity = hf::json_get_str_value(now_obj, "humidity");
    };

    if (hf::query_func_frame(url_host, url_path, func, errors, http_headers)) {
        _realtimeWeather = data;
        return true;
    } else {
        errors.push_back(L"QueryRealtimeWeather failed");
        return false;
    }
}

bool DataApiHefengWeather::QueryRealtimeAirQuality(const std::wstring &query, WStringList &errors)
{
    HFW_CHECK;

    _realtimeAirQuality = RealtimeAirQuality();
    const auto &dm = CDataManager::Instance();

    std::string url_host = std::format("https://{}", utils::wide_char2multi_byte(config.ApiHost.c_str()));
    std::string url_path;
    httplib::Headers http_headers;

    if (config.AuthViaJWT) {
        auto jwt = hf::generate_jwt(config, errors);
        if (jwt.empty()) {
            return false;
        }

        http_headers.emplace(std::make_pair("Authorization", std::format("Bearer {}", jwt)));

        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/air/now?location={}&lang={}",
                        query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    } else {
        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/air/now?key={}&location={}&lang={}",
                        config.AppKey, query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    }

    RealtimeAirQuality data;
    auto func = [&data](yyjson_val *j_val) {
        auto *now_obj = yyjson_obj_get(j_val, "now");

        data.PublishTime = hf::json_get_str_value(now_obj, "pubTime").substr(11, 5);
        data.AQI = hf::json_get_str_value(now_obj, "aqi");
        data.Level = hf::json_get_str_value(now_obj, "level");
        data.Category = hf::json_get_str_value(now_obj, "category");
        data.PM2p5 = hf::json_get_str_value(now_obj, "pm2p5");
        data.PM10 = hf::json_get_str_value(now_obj, "pm10");
    };

    if (hf::query_func_frame(url_host, url_path, func, errors, http_headers)) {
        _realtimeAirQuality = data;
        _airQualityDataOutdated = false;

        return true;
    } else {
        errors.push_back(L"QueryRealtimeAirQuality failed");
        return false;
    }
}

bool DataApiHefengWeather::QueryForecastWeather(const std::wstring &query, WStringList &errors)
{
    HFW_CHECK;

    _forcastWeatherTD = ForcastWeather();
    _forcastWeatherTM = ForcastWeather();
    _forcastWeatherDATM = ForcastWeather();
    const auto &dm = CDataManager::Instance();

    std::string url_host = std::format("https://{}", utils::wide_char2multi_byte(config.ApiHost.c_str()));
    std::string url_path;
    httplib::Headers http_headers;

    if (config.AuthViaJWT) {
        auto jwt = hf::generate_jwt(config, errors);
        if (jwt.empty()) {
            return false;
        }

        http_headers.emplace(std::make_pair("Authorization", std::format("Bearer {}", jwt)));

        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/weather/3d?location={}&lang={}",
                        query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    } else {
        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/weather/3d?key={}&location={}&lang={}",
                        config.AppKey, query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    }

    ForcastWeather td, tm, datm;
    auto func = [&td, &tm, &datm](yyjson_val *j_val) {
        auto *daily_arr = yyjson_obj_get(j_val, "daily");

        auto get_daily_info = [](yyjson_val *j_val, ForcastWeather &w) {
            w.TemperatureMax = hf::json_get_str_value(j_val, "tempMax");
            w.TemperatureMin = hf::json_get_str_value(j_val, "tempMin");
            w.WeatherDay = hf::json_get_str_value(j_val, "textDay");
            w.WeatherNight = hf::json_get_str_value(j_val, "textNight");
            w.WeatherCodeDay = hf::json_get_str_value(j_val, "iconDay");
            w.WeatherCodeNight = hf::json_get_str_value(j_val, "iconNight");
            w.UVIndex = hf::json_get_str_value(j_val, "uvIndex");
            w.Humidity = hf::json_get_str_value(j_val, "humidity");
        };

        get_daily_info(yyjson_arr_get(daily_arr, 0), td);
        get_daily_info(yyjson_arr_get(daily_arr, 1), tm);
        get_daily_info(yyjson_arr_get(daily_arr, 2), datm);
    };

    if (hf::query_func_frame(url_host, url_path, func, errors, http_headers)) {
        _forcastWeatherTD = td;
        _forcastWeatherTM = tm;
        _forcastWeatherDATM = datm;

        return true;
    } else {
        errors.push_back(L"QueryForecastWeather failed");
        return false;
    }
}

bool DataApiHefengWeather::QueryWeatherAlerts(const std::wstring &query, WStringList &errors)
{
    HFW_CHECK;

    _weatherAlerts.clear();
    const auto &dm = CDataManager::Instance();

    std::string url_host = std::format("https://{}", utils::wide_char2multi_byte(config.ApiHost.c_str()));
    std::string url_path;
    httplib::Headers http_headers;

    if (config.AuthViaJWT) {
        auto jwt = hf::generate_jwt(config, errors);
        if (jwt.empty()) {
            return false;
        }

        http_headers.emplace(std::make_pair("Authorization", std::format("Bearer {}", jwt)));

        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/warning/now?location={}&lang={}",
                        query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    } else {
        url_path = utils::wide_char2multi_byte(
            std::format(L"/v7/warning/now?key={}&location={}&lang={}",
                        config.AppKey, query, dm.StringRes(IDS_HFW_LANG).GetString()).c_str()
        );
    }

    WeatherAlertList data;
    auto func = [&data](yyjson_val *j_val) {
        auto *alert_arr = yyjson_obj_get(j_val, "warning");
        auto num_alerts = yyjson_arr_size(alert_arr);

        auto get_alert_info = [](yyjson_val *j_val) {
            WeatherAlert alert;
            alert.Type = hf::json_get_str_value(j_val, "type");
            alert.TypeName = hf::json_get_str_value(j_val, "typeName");
            alert.Level = hf::json_get_str_value(j_val, "level");
            alert.Title = hf::json_get_str_value(j_val, "title");
            alert.Text = hf::json_get_str_value(j_val, "text");
            alert.Severity = hf::json_get_str_value(j_val, "severity");
            alert.SeverityColor = hf::json_get_str_value(j_val, "severityColor");
            alert.PublishTime = hf::json_get_str_value(j_val, "pubTime").substr(0, 16).replace(10, 1, L" ");
            return alert;
        };

        for (decltype(num_alerts) i = 0; i < num_alerts; ++i)
            data.push_back(get_alert_info(yyjson_arr_get(alert_arr, i)));
    };

    if (hf::query_func_frame(url_host, url_path, func, errors, http_headers)) {
        _weatherAlerts = data;
        _alertsDataOutdated = false;

        return true;
    } else {
        errors.push_back(L"QueryWeatherAlerts failed");
        return false;
    }
}
