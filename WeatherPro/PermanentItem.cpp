#include "pch.h"
#include "PermanentItem.h"
#include "DataManager.h"

#include <unordered_map>
#include <format>

namespace pit
{
    struct ItemData
    {
        std::wstring Name;
        std::wstring Id;
        std::wstring LabelText;
        std::wstring ValueSampleText;
    };

    const ItemData& get_item_data(PermanmentItemType t) {
        static std::unordered_map<PermanmentItemType, ItemData> data_map;

        const CDataManager &dm = CDataManager::Instance();
    
        if (!data_map.contains(t)){
            ItemData item_data;
            switch (t) {
                case PermanmentItemType::RT_Humidity:
                    item_data.Name = dm.StringRes(IDS_PI_RT_HUMIDITY);
                    item_data.Id = L"FxOP3401";
                    item_data.ValueSampleText = L"100%";
                    break;
                case PermanmentItemType::RT_Wind:
                    item_data.Name = dm.StringRes(IDS_PI_RT_WIND);
                    item_data.Id = L"FxOP3402";
                    item_data.ValueSampleText = dm.StringRes(IDS_PI_WIND_SAMPLE);
                    break;
                case PermanmentItemType::RT_AQI:
                    item_data.Name = dm.StringRes(IDS_PI_RT_AQI);
                    item_data.Id = L"FxOP3403";
                    item_data.ValueSampleText = dm.StringRes(IDS_PI_AIR_SAMPLE);
                    break;
                case PermanmentItemType::RT_PM2P5:
                    item_data.Name = dm.StringRes(IDS_PI_RT_PM2P5);
                    item_data.Id = L"FxOP3404";
                    item_data.ValueSampleText = L"999";
                    break;
                case PermanmentItemType::RT_PM10:
                    item_data.Name = dm.StringRes(IDS_PI_RT_PM10);
                    item_data.Id = L"FxOP3405";
                    item_data.ValueSampleText = L"999";
                    break;
                default:
                    break;
            }

            item_data.LabelText = std::format(L"{}: ", item_data.Name);
            data_map.insert({ t, std::move(item_data) });
        }

        return data_map.at(t);
    }
}

CPermanmentItem::CPermanmentItem(PermanmentItemType t) :
    type(t)
{
}

const wchar_t* CPermanmentItem::GetItemName() const {
    return pit::get_item_data(type).Name.c_str();
}

const wchar_t* CPermanmentItem::GetItemId() const {
    return pit::get_item_data(type).Id.c_str();
}

const wchar_t* CPermanmentItem::GetItemLableText() const {
    return pit::get_item_data(type).LabelText.c_str();
}

const wchar_t* CPermanmentItem::GetItemValueSampleText() const {
    return pit::get_item_data(type).ValueSampleText.c_str();
}

const wchar_t* CPermanmentItem::GetItemValueText() const {
    const auto &dm = CDataManager::Instance();
    auto api = dm.GetCurrentApi();

    static std::wstring value_text;

    if (api) {
        switch (type) {
            case PermanmentItemType::RT_Humidity:
                value_text = api->GetHumidity(EWeatherInfoType::WEATHER_REALTIME);
                break;
            case PermanmentItemType::RT_AQI:
                value_text = api->GetAQI(EWeatherInfoType::WEATHER_REALTIME);
                break;
            case PermanmentItemType::RT_Wind:
                value_text = api->GetWind(EWeatherInfoType::WEATHER_REALTIME);
                break;
            case PermanmentItemType::RT_PM2P5:
                value_text = api->GetPM2p5(EWeatherInfoType::WEATHER_REALTIME);
                break;
            case PermanmentItemType::RT_PM10:
                value_text = api->GetPM10(EWeatherInfoType::WEATHER_REALTIME);
                break;
            default:
                break;
        }
    } else {
        value_text = L"-";
    }

    return value_text.c_str();
}
