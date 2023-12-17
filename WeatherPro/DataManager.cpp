#include "pch.h"
#include "DataManager.h"
#include "resource.h"
#include "SimpleIni.h"

#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>

namespace icon
{
    std::map<UINT, HICON> icons;

    std::wstring code_converting_hfw2wcc(const std::wstring &code)
    {
        static const std::map<std::wstring, std::wstring> dmap{
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
            {L"999", L"99"},
        };

        auto itr = dmap.find(code);
        if (itr != dmap.end())
            return itr->second;
        else
            return L"99";
    }

    // using codes provided by weather.com.cn(WCC) as the basic codes
    std::wstring to_basic_code(DataApiType api_type, const std::wstring &code)
    {
        switch (api_type)
        {
        default:
        case DataApiType::API_WeatherComCnSpider:
            return code;

        case DataApiType::API_HefengWeather:
            return code_converting_hfw2wcc(code);
        }
    }

    class IconSet
    {
    public:
        // using codes provided by weather.com.cn(WCC) as the basic codes
        virtual UINT get_icon_id_by_basic_code(const std::wstring &code) = 0;
        virtual UINT get_icon_id(DataApiType api_type, const std::wstring &code) = 0;
    };

    using IconSetSPtr = std::shared_ptr<IconSet>;

    class IconSetWccBlue : public IconSet
    {
    public:
        UINT get_icon_id_by_basic_code(const std::wstring &code) override
        {
            static const std::map<std::wstring, UINT> dmap{
                {L"d00", IDI_ICON_B_D00},
                {L"d01", IDI_ICON_B_D01},
                {L"d03", IDI_ICON_B_D03},
                {L"d13", IDI_ICON_B_D13},
                {L"n00", IDI_ICON_B_N00},
                {L"n01", IDI_ICON_B_N01},
                {L"n03", IDI_ICON_B_N03},
                {L"n13", IDI_ICON_B_N13},
                {L"02", IDI_ICON_B_A02},
                {L"04", IDI_ICON_B_A04},
                {L"05", IDI_ICON_B_A05},
                {L"06", IDI_ICON_B_A06},
                {L"07", IDI_ICON_B_A07},
                {L"08", IDI_ICON_B_A08},
                {L"09", IDI_ICON_B_A09},
                {L"10", IDI_ICON_B_A10},
                {L"11", IDI_ICON_B_A11},
                {L"12", IDI_ICON_B_A12},
                {L"14", IDI_ICON_B_A14},
                {L"15", IDI_ICON_B_A15},
                {L"16", IDI_ICON_B_A16},
                {L"17", IDI_ICON_B_A17},
                {L"18", IDI_ICON_B_A18},
                {L"19", IDI_ICON_B_A19},
                {L"20", IDI_ICON_B_A20},
                {L"29", IDI_ICON_B_A29},
                {L"30", IDI_ICON_B_A30},
                {L"31", IDI_ICON_B_A31},
                {L"53", IDI_ICON_B_A53},
                {L"54", IDI_ICON_B_A54},
                {L"55", IDI_ICON_B_A55},
                {L"56", IDI_ICON_B_A56},
                {L"99", IDI_ICON_B_A99},
                {L"21", IDI_ICON_B_A08},
                {L"22", IDI_ICON_B_A09},
                {L"23", IDI_ICON_B_A10},
                {L"24", IDI_ICON_B_A11},
                {L"25", IDI_ICON_B_A12},
                {L"26", IDI_ICON_B_A15},
                {L"27", IDI_ICON_B_A16},
                {L"28", IDI_ICON_B_A17},
                {L"32", IDI_ICON_B_A18},
                {L"49", IDI_ICON_B_A18},
                {L"57", IDI_ICON_B_A18},
                {L"58", IDI_ICON_B_A18},
                {L"97", IDI_ICON_B_A08},
                {L"98", IDI_ICON_B_A15},
                {L"301", IDI_ICON_B_A08},
                {L"302", IDI_ICON_B_A15},
            };

            auto itr = dmap.find(code);
            if (itr != dmap.end())
                return itr->second;
            else if (code.size() > 1)
            {
                auto itr2 = dmap.find(code.substr(1));
                if (itr2 != dmap.end())
                    return itr2->second;
            }

            return IDI_ICON_B_A99;
        }

        UINT get_icon_id(DataApiType api_type, const std::wstring &code) override
        {
            return get_icon_id_by_basic_code(to_basic_code(api_type, code));
        }
    };

    class IconSetWccWhite : public IconSet
    {
    public:
        UINT get_icon_id_by_basic_code(const std::wstring &code) override
        {
            static const std::map<std::wstring, UINT> dmap{
                {L"d00", IDI_ICON_W_D00},
                {L"d01", IDI_ICON_W_D01},
                {L"d03", IDI_ICON_W_D03},
                {L"d13", IDI_ICON_W_D13},
                {L"n00", IDI_ICON_W_N00},
                {L"n01", IDI_ICON_W_N01},
                {L"n03", IDI_ICON_W_N03},
                {L"n13", IDI_ICON_W_N13},
                {L"02", IDI_ICON_W_A02},
                {L"04", IDI_ICON_W_A04},
                {L"05", IDI_ICON_W_A05},
                {L"06", IDI_ICON_W_A06},
                {L"07", IDI_ICON_W_A07},
                {L"08", IDI_ICON_W_A08},
                {L"09", IDI_ICON_W_A09},
                {L"10", IDI_ICON_W_A10},
                {L"11", IDI_ICON_W_A11},
                {L"12", IDI_ICON_W_A12},
                {L"14", IDI_ICON_W_A14},
                {L"15", IDI_ICON_W_A15},
                {L"16", IDI_ICON_W_A16},
                {L"17", IDI_ICON_W_A17},
                {L"18", IDI_ICON_W_A18},
                {L"19", IDI_ICON_W_A19},
                {L"20", IDI_ICON_W_A20},
                {L"29", IDI_ICON_W_A29},
                {L"30", IDI_ICON_W_A30},
                {L"31", IDI_ICON_W_A31},
                {L"53", IDI_ICON_W_A53},
                {L"54", IDI_ICON_W_A54},
                {L"55", IDI_ICON_W_A55},
                {L"56", IDI_ICON_W_A56},
                {L"99", IDI_ICON_W_A99},
                {L"21", IDI_ICON_W_A08},
                {L"22", IDI_ICON_W_A09},
                {L"23", IDI_ICON_W_A10},
                {L"24", IDI_ICON_W_A11},
                {L"25", IDI_ICON_W_A12},
                {L"26", IDI_ICON_W_A15},
                {L"27", IDI_ICON_W_A16},
                {L"28", IDI_ICON_W_A17},
                {L"32", IDI_ICON_W_A18},
                {L"49", IDI_ICON_W_A18},
                {L"57", IDI_ICON_W_A18},
                {L"58", IDI_ICON_W_A18},
                {L"97", IDI_ICON_W_A08},
                {L"98", IDI_ICON_W_A15},
                {L"301", IDI_ICON_W_A08},
                {L"302", IDI_ICON_W_A15},
            };

            auto itr = dmap.find(code);
            if (itr != dmap.end())
                return itr->second;
            else if (code.size() > 1)
            {
                auto itr2 = dmap.find(code.substr(1));
                if (itr2 != dmap.end())
                    return itr2->second;
            }

            return IDI_ICON_W_A99;
        }

        UINT get_icon_id(DataApiType api_type, const std::wstring &code) override
        {
            return get_icon_id_by_basic_code(to_basic_code(api_type, code));
        }
    };

    class IconSetHfwFill : public IconSet
    {
    public:
        UINT get_icon_id_by_basic_code(const std::wstring &code) override
        {
            static const std::map<std::wstring, UINT> dmap{
                {L"d00", IDI_ICON_HFW_FILL_100},
                {L"d01", IDI_ICON_HFW_FILL_101},
                {L"d03", IDI_ICON_HFW_FILL_300},
                {L"d13", IDI_ICON_HFW_FILL_407},
                {L"n00", IDI_ICON_HFW_FILL_150},
                {L"n01", IDI_ICON_HFW_FILL_151},
                {L"n03", IDI_ICON_HFW_FILL_350},
                {L"n13", IDI_ICON_HFW_FILL_457},
                {L"02", IDI_ICON_HFW_FILL_104},
                {L"04", IDI_ICON_HFW_FILL_302},
                {L"05", IDI_ICON_HFW_FILL_304},
                {L"06", IDI_ICON_HFW_FILL_404},
                {L"07", IDI_ICON_HFW_FILL_305},
                {L"08", IDI_ICON_HFW_FILL_306},
                {L"09", IDI_ICON_HFW_FILL_307},
                {L"10", IDI_ICON_HFW_FILL_310},
                {L"11", IDI_ICON_HFW_FILL_311},
                {L"12", IDI_ICON_HFW_FILL_312},
                {L"14", IDI_ICON_HFW_FILL_400},
                {L"15", IDI_ICON_HFW_FILL_401},
                {L"16", IDI_ICON_HFW_FILL_402},
                {L"17", IDI_ICON_HFW_FILL_403},
                {L"18", IDI_ICON_HFW_FILL_501},
                {L"19", IDI_ICON_HFW_FILL_313},
                {L"20", IDI_ICON_HFW_FILL_507},
                {L"29", IDI_ICON_HFW_FILL_504},
                {L"30", IDI_ICON_HFW_FILL_503},
                {L"31", IDI_ICON_HFW_FILL_508},
                {L"53", IDI_ICON_HFW_FILL_502},
                {L"54", IDI_ICON_HFW_FILL_511},
                {L"55", IDI_ICON_HFW_FILL_512},
                {L"56", IDI_ICON_HFW_FILL_513},
                {L"99", IDI_ICON_HFW_FILL_999},
                {L"21", IDI_ICON_HFW_FILL_314},
                {L"22", IDI_ICON_HFW_FILL_315},
                {L"23", IDI_ICON_HFW_FILL_316},
                {L"24", IDI_ICON_HFW_FILL_317},
                {L"25", IDI_ICON_HFW_FILL_318},
                {L"26", IDI_ICON_HFW_FILL_408},
                {L"27", IDI_ICON_HFW_FILL_409},
                {L"28", IDI_ICON_HFW_FILL_410},
                {L"32", IDI_ICON_HFW_FILL_509},
                {L"49", IDI_ICON_HFW_FILL_510},
                {L"57", IDI_ICON_HFW_FILL_514},
                {L"58", IDI_ICON_HFW_FILL_515},
                {L"97", IDI_ICON_HFW_FILL_399},
                {L"98", IDI_ICON_HFW_FILL_499},
                {L"301", IDI_ICON_HFW_FILL_399},
                {L"302", IDI_ICON_HFW_FILL_499},
            };

            auto itr = dmap.find(code);
            if (itr != dmap.end())
                return itr->second;
            else if (code.size() > 1)
            {
                auto itr2 = dmap.find(code.substr(1));
                if (itr2 != dmap.end())
                    return itr2->second;
            }

            return IDI_ICON_HFW_FILL_999;
        }

        UINT get_icon_id_by_hfw_code(const std::wstring &code)
        {
            static const std::map<std::wstring, UINT> dmap{
                {L"100", IDI_ICON_HFW_FILL_100},
                {L"101", IDI_ICON_HFW_FILL_101},
                {L"102", IDI_ICON_HFW_FILL_102},
                {L"103", IDI_ICON_HFW_FILL_103},
                {L"104", IDI_ICON_HFW_FILL_104},
                {L"150", IDI_ICON_HFW_FILL_150},
                {L"151", IDI_ICON_HFW_FILL_151},
                {L"152", IDI_ICON_HFW_FILL_152},
                {L"153", IDI_ICON_HFW_FILL_153},
                {L"154", IDI_ICON_HFW_FILL_104},
                {L"300", IDI_ICON_HFW_FILL_300},
                {L"301", IDI_ICON_HFW_FILL_301},
                {L"302", IDI_ICON_HFW_FILL_302},
                {L"303", IDI_ICON_HFW_FILL_303},
                {L"304", IDI_ICON_HFW_FILL_304},
                {L"305", IDI_ICON_HFW_FILL_305},
                {L"306", IDI_ICON_HFW_FILL_306},
                {L"307", IDI_ICON_HFW_FILL_307},
                {L"308", IDI_ICON_HFW_FILL_308},
                {L"309", IDI_ICON_HFW_FILL_309},
                {L"310", IDI_ICON_HFW_FILL_310},
                {L"311", IDI_ICON_HFW_FILL_311},
                {L"312", IDI_ICON_HFW_FILL_312},
                {L"313", IDI_ICON_HFW_FILL_313},
                {L"314", IDI_ICON_HFW_FILL_314},
                {L"315", IDI_ICON_HFW_FILL_315},
                {L"316", IDI_ICON_HFW_FILL_316},
                {L"317", IDI_ICON_HFW_FILL_317},
                {L"318", IDI_ICON_HFW_FILL_318},
                {L"350", IDI_ICON_HFW_FILL_350},
                {L"351", IDI_ICON_HFW_FILL_351},
                {L"399", IDI_ICON_HFW_FILL_399},
                {L"400", IDI_ICON_HFW_FILL_400},
                {L"401", IDI_ICON_HFW_FILL_401},
                {L"402", IDI_ICON_HFW_FILL_402},
                {L"403", IDI_ICON_HFW_FILL_403},
                {L"404", IDI_ICON_HFW_FILL_404},
                {L"405", IDI_ICON_HFW_FILL_405},
                {L"406", IDI_ICON_HFW_FILL_406},
                {L"407", IDI_ICON_HFW_FILL_407},
                {L"408", IDI_ICON_HFW_FILL_408},
                {L"409", IDI_ICON_HFW_FILL_409},
                {L"410", IDI_ICON_HFW_FILL_410},
                {L"456", IDI_ICON_HFW_FILL_456},
                {L"457", IDI_ICON_HFW_FILL_457},
                {L"499", IDI_ICON_HFW_FILL_499},
                {L"500", IDI_ICON_HFW_FILL_500},
                {L"501", IDI_ICON_HFW_FILL_501},
                {L"502", IDI_ICON_HFW_FILL_502},
                {L"503", IDI_ICON_HFW_FILL_503},
                {L"504", IDI_ICON_HFW_FILL_504},
                {L"507", IDI_ICON_HFW_FILL_507},
                {L"508", IDI_ICON_HFW_FILL_508},
                {L"509", IDI_ICON_HFW_FILL_509},
                {L"510", IDI_ICON_HFW_FILL_510},
                {L"511", IDI_ICON_HFW_FILL_511},
                {L"512", IDI_ICON_HFW_FILL_512},
                {L"513", IDI_ICON_HFW_FILL_513},
                {L"514", IDI_ICON_HFW_FILL_514},
                {L"515", IDI_ICON_HFW_FILL_515},
                {L"999", IDI_ICON_HFW_FILL_999},
            };

            auto itr = dmap.find(code);
            if (itr != dmap.end())
                return itr->second;

            return IDI_ICON_HFW_FILL_999;
        }

        UINT get_icon_id(DataApiType api_type, const std::wstring &code) override
        {
            if (api_type == DataApiType::API_HefengWeather)
                return get_icon_id_by_hfw_code(code);
            else
                return get_icon_id_by_basic_code(to_basic_code(api_type, code));
        }
    };

    class IconSetHfwHollow : public IconSet
    {
    public:
        UINT get_icon_id_by_basic_code(const std::wstring &code) override
        {
            static const std::map<std::wstring, UINT> dmap{
                {L"d00", IDI_ICON_HFW_HOLLOW_100},
                {L"d01", IDI_ICON_HFW_HOLLOW_101},
                {L"d03", IDI_ICON_HFW_HOLLOW_300},
                {L"d13", IDI_ICON_HFW_HOLLOW_407},
                {L"n00", IDI_ICON_HFW_HOLLOW_150},
                {L"n01", IDI_ICON_HFW_HOLLOW_151},
                {L"n03", IDI_ICON_HFW_HOLLOW_350},
                {L"n13", IDI_ICON_HFW_HOLLOW_457},
                {L"02", IDI_ICON_HFW_HOLLOW_104},
                {L"04", IDI_ICON_HFW_HOLLOW_302},
                {L"05", IDI_ICON_HFW_HOLLOW_304},
                {L"06", IDI_ICON_HFW_HOLLOW_404},
                {L"07", IDI_ICON_HFW_HOLLOW_305},
                {L"08", IDI_ICON_HFW_HOLLOW_306},
                {L"09", IDI_ICON_HFW_HOLLOW_307},
                {L"10", IDI_ICON_HFW_HOLLOW_310},
                {L"11", IDI_ICON_HFW_HOLLOW_311},
                {L"12", IDI_ICON_HFW_HOLLOW_312},
                {L"14", IDI_ICON_HFW_HOLLOW_400},
                {L"15", IDI_ICON_HFW_HOLLOW_401},
                {L"16", IDI_ICON_HFW_HOLLOW_402},
                {L"17", IDI_ICON_HFW_HOLLOW_403},
                {L"18", IDI_ICON_HFW_HOLLOW_501},
                {L"19", IDI_ICON_HFW_HOLLOW_313},
                {L"20", IDI_ICON_HFW_HOLLOW_507},
                {L"29", IDI_ICON_HFW_HOLLOW_504},
                {L"30", IDI_ICON_HFW_HOLLOW_503},
                {L"31", IDI_ICON_HFW_HOLLOW_508},
                {L"53", IDI_ICON_HFW_HOLLOW_502},
                {L"54", IDI_ICON_HFW_HOLLOW_511},
                {L"55", IDI_ICON_HFW_HOLLOW_512},
                {L"56", IDI_ICON_HFW_HOLLOW_513},
                {L"99", IDI_ICON_HFW_HOLLOW_999},
                {L"21", IDI_ICON_HFW_HOLLOW_314},
                {L"22", IDI_ICON_HFW_HOLLOW_315},
                {L"23", IDI_ICON_HFW_HOLLOW_316},
                {L"24", IDI_ICON_HFW_HOLLOW_317},
                {L"25", IDI_ICON_HFW_HOLLOW_318},
                {L"26", IDI_ICON_HFW_HOLLOW_408},
                {L"27", IDI_ICON_HFW_HOLLOW_409},
                {L"28", IDI_ICON_HFW_HOLLOW_410},
                {L"32", IDI_ICON_HFW_HOLLOW_509},
                {L"49", IDI_ICON_HFW_HOLLOW_510},
                {L"57", IDI_ICON_HFW_HOLLOW_514},
                {L"58", IDI_ICON_HFW_HOLLOW_515},
                {L"97", IDI_ICON_HFW_HOLLOW_399},
                {L"98", IDI_ICON_HFW_HOLLOW_499},
                {L"301", IDI_ICON_HFW_HOLLOW_399},
                {L"302", IDI_ICON_HFW_HOLLOW_499},
            };

            auto itr = dmap.find(code);
            if (itr != dmap.end())
                return itr->second;
            else if (code.size() > 1)
            {
                auto itr2 = dmap.find(code.substr(1));
                if (itr2 != dmap.end())
                    return itr2->second;
            }

            return IDI_ICON_HFW_HOLLOW_999;
        }

        UINT get_icon_id_by_hfw_code(const std::wstring &code)
        {
            static const std::map<std::wstring, UINT> dmap{
                {L"100", IDI_ICON_HFW_HOLLOW_100},
                {L"101", IDI_ICON_HFW_HOLLOW_101},
                {L"102", IDI_ICON_HFW_HOLLOW_102},
                {L"103", IDI_ICON_HFW_HOLLOW_103},
                {L"104", IDI_ICON_HFW_HOLLOW_104},
                {L"150", IDI_ICON_HFW_HOLLOW_150},
                {L"151", IDI_ICON_HFW_HOLLOW_151},
                {L"152", IDI_ICON_HFW_HOLLOW_152},
                {L"153", IDI_ICON_HFW_HOLLOW_153},
                {L"154", IDI_ICON_HFW_HOLLOW_104},
                {L"300", IDI_ICON_HFW_HOLLOW_300},
                {L"301", IDI_ICON_HFW_HOLLOW_301},
                {L"302", IDI_ICON_HFW_HOLLOW_302},
                {L"303", IDI_ICON_HFW_HOLLOW_303},
                {L"304", IDI_ICON_HFW_HOLLOW_304},
                {L"305", IDI_ICON_HFW_HOLLOW_305},
                {L"306", IDI_ICON_HFW_HOLLOW_306},
                {L"307", IDI_ICON_HFW_HOLLOW_307},
                {L"308", IDI_ICON_HFW_HOLLOW_308},
                {L"309", IDI_ICON_HFW_HOLLOW_309},
                {L"310", IDI_ICON_HFW_HOLLOW_310},
                {L"311", IDI_ICON_HFW_HOLLOW_311},
                {L"312", IDI_ICON_HFW_HOLLOW_312},
                {L"313", IDI_ICON_HFW_HOLLOW_313},
                {L"314", IDI_ICON_HFW_HOLLOW_314},
                {L"315", IDI_ICON_HFW_HOLLOW_315},
                {L"316", IDI_ICON_HFW_HOLLOW_316},
                {L"317", IDI_ICON_HFW_HOLLOW_317},
                {L"318", IDI_ICON_HFW_HOLLOW_318},
                {L"350", IDI_ICON_HFW_HOLLOW_350},
                {L"351", IDI_ICON_HFW_HOLLOW_351},
                {L"399", IDI_ICON_HFW_HOLLOW_399},
                {L"400", IDI_ICON_HFW_HOLLOW_400},
                {L"401", IDI_ICON_HFW_HOLLOW_401},
                {L"402", IDI_ICON_HFW_HOLLOW_402},
                {L"403", IDI_ICON_HFW_HOLLOW_403},
                {L"404", IDI_ICON_HFW_HOLLOW_404},
                {L"405", IDI_ICON_HFW_HOLLOW_405},
                {L"406", IDI_ICON_HFW_HOLLOW_406},
                {L"407", IDI_ICON_HFW_HOLLOW_407},
                {L"408", IDI_ICON_HFW_HOLLOW_408},
                {L"409", IDI_ICON_HFW_HOLLOW_409},
                {L"410", IDI_ICON_HFW_HOLLOW_410},
                {L"456", IDI_ICON_HFW_HOLLOW_456},
                {L"457", IDI_ICON_HFW_HOLLOW_457},
                {L"499", IDI_ICON_HFW_HOLLOW_499},
                {L"500", IDI_ICON_HFW_HOLLOW_500},
                {L"501", IDI_ICON_HFW_HOLLOW_501},
                {L"502", IDI_ICON_HFW_HOLLOW_502},
                {L"503", IDI_ICON_HFW_HOLLOW_503},
                {L"504", IDI_ICON_HFW_HOLLOW_504},
                {L"507", IDI_ICON_HFW_HOLLOW_507},
                {L"508", IDI_ICON_HFW_HOLLOW_508},
                {L"509", IDI_ICON_HFW_HOLLOW_509},
                {L"510", IDI_ICON_HFW_HOLLOW_510},
                {L"511", IDI_ICON_HFW_HOLLOW_511},
                {L"512", IDI_ICON_HFW_HOLLOW_512},
                {L"513", IDI_ICON_HFW_HOLLOW_513},
                {L"514", IDI_ICON_HFW_HOLLOW_514},
                {L"515", IDI_ICON_HFW_HOLLOW_515},
                {L"999", IDI_ICON_HFW_HOLLOW_999},
            };

            auto itr = dmap.find(code);
            if (itr != dmap.end())
                return itr->second;

            return IDI_ICON_HFW_HOLLOW_999;
        }

        UINT get_icon_id(DataApiType api_type, const std::wstring &code) override
        {
            if (api_type == DataApiType::API_HefengWeather)
                return get_icon_id_by_hfw_code(code);
            else
                return get_icon_id_by_basic_code(to_basic_code(api_type, code));
        }
    };

    IconSetSPtr get_icon_set(IconType icon_type)
    {
        switch (icon_type)
        {
        case IconType::IT_WCC_BULE:
            return std::make_shared<IconSetWccBlue>();
        case IconType::IT_WCC_WHITE:
            return std::make_shared<IconSetWccWhite>();
        case IconType::IT_HFW_FILL:
            return std::make_shared<IconSetHfwFill>();
        case IconType::IT_HFW_HOLLOW:
            return std::make_shared<IconSetHfwHollow>();

        default:
            return nullptr;
        }
    }
    
    HICON get_icon(DataApiType api_type, const std::wstring &code, IconType icon_type)
    {
        auto icon_set = get_icon_set(icon_type);
        if (!icon_set) return nullptr;

        auto icon_id = icon_set->get_icon_id(api_type, code);

        if (icons.count(icon_id) == 0)
        {
            const auto &dm = CDataManager::Instance();
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(icon_id), IMAGE_ICON,
                                           dm.DPI(16), dm.DPI(16), 0);

            icons[icon_id] = hIcon;
        }

        return icons[icon_id];
    }
}

std::mutex g_weather_update_nutex;

SConfiguration::SConfiguration() :
    m_api_type(DataApiType::API_HefengWeather),
    m_wit(EWeatherInfoType::WEATHER_REALTIME),
    m_update_frequency(UpdateFrequency::UF_2T1H),
    m_icon_type(IconType::IT_WCC_BULE),
    m_show_weather_icon(true),
    m_show_weather_in_tooltips(true),
    m_show_brief_rt_weather_info(false),
    m_show_weather_alerts(true),
    m_show_brief_weather_alert_info(true),
    m_show_error_info(false)
{}

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
    HDC hDC = ::GetDC(HWND_DESKTOP);
    m_dpi = ::GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);

    m_api_wccs = std::make_shared<DataApiWeatherComCnSpider>();
    m_api_hfw = std::make_shared<DataApiHefengWeather>();

    m_lang_id = GetThreadUILanguage();  // 默认为系统语言
}

CDataManager::~CDataManager()
{
    SaveConfigs();
}

const CDataManager& CDataManager::Instance()
{
    return m_instance;
}

CDataManager& CDataManager::InstanceRef()
{
    return m_instance;
}

const CString& CDataManager::StringRes(UINT id) const
{
    if (m_string_resources.count(id) == 0)
    {
        if (m_lang_id != GetThreadUILanguage())
            SetThreadUILanguage(m_lang_id);

        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_resources[id].LoadString(id);
    }

    return m_string_resources[id];
}

int CDataManager::DPI(int pixel) const
{
    return m_dpi * pixel / 96;
}

float CDataManager::DPIF(float pixel) const
{
    return m_dpi * pixel / 96;
}

int CDataManager::RDPI(int pixel) const
{
    return pixel * 96 / m_dpi;
}

void CDataManager::SetCurrentCityInfo(SCityInfo info)
{
    m_currentCityInfo = info;
}

const SCityInfo& CDataManager::GetCurrentCityInfo() const
{
    return m_currentCityInfo;
}

DataApiPtr CDataManager::GetCurrentApi() const
{
    if (m_config.m_api_type == DataApiType::API_WeatherComCnSpider)
        return m_api_wccs;
    else if (m_config.m_api_type == DataApiType::API_HefengWeather)
        return m_api_hfw;
    else
        return nullptr;
}

void CDataManager::_updateWeather(WeatherInfoUpdatedCallback callback)
{
    std::lock_guard<std::mutex> guard(g_weather_update_nutex);

    std::shared_ptr<DataAPI> current_api{ GetCurrentApi() };
    if (current_api != nullptr)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (current_api->UpdateWeather())
                break;

            std::this_thread::sleep_for(std::chrono::seconds(3));
        }

        RefreshWeatherInfoCache();

        if (callback != nullptr)
        {
            callback(GetTooptipInfo());
        }
    }
}

void CDataManager::UpdateWeather(WeatherInfoUpdatedCallback callback /* = nullptr */)
{
    std::thread t(&CDataManager::_updateWeather, this, callback);
    t.detach();
}

std::wstring CDataManager::GetWeatherTemperature() const
{
    return m_weather_info_cache.WeatherTemperature;
}

std::wstring CDataManager::GetTooptipInfo() const
{
    return m_weather_info_cache.TooltipInfo;
}

SConfiguration& CDataManager::GetConfig()
{
    return m_config;
}

const SConfiguration& CDataManager::GetConfig() const
{
    return m_config;
}

void CDataManager::_setLangID(const std::wstring &cfg_dir)
{
    static const WORD langid_en_us = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    static const WORD langid_zh_cn = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);

    // 插件配置文件所在文件夹的上一级为主程序配置文件路径
    auto pos = cfg_dir.rfind(L"plugins");
    if (pos == std::wstring::npos)
        return;

    auto tm_cfg_dir = cfg_dir.substr(0, pos);
    auto tm_cfg_file = tm_cfg_dir + L"config.ini";

    CSimpleIni tm_ini;
    tm_ini.SetUnicode();
    tm_ini.LoadFile(tm_cfg_file.c_str());

    auto tm_lang_id = tm_ini.GetLongValue(L"general", L"language", 0);
    if (tm_lang_id == 0)  // 跟随系统
    {
        auto p_lang_id = PRIMARYLANGID(m_lang_id);
        if (p_lang_id != LANG_CHINESE)
            m_lang_id = langid_en_us;
        else
            m_lang_id = langid_zh_cn;
    } else if (tm_lang_id == 1)  // 英语
        m_lang_id = langid_en_us;
    else if (tm_lang_id == 2 || tm_lang_id == 3)  // 汉语
        m_lang_id = langid_zh_cn;
    else  // 处理意外值
        m_lang_id = langid_en_us;
}

void CDataManager::LoadConfigs(const std::wstring &cfg_dir)
{
    _setLangID(cfg_dir);

    const auto &ch = cfg_dir.back();
    if (ch == L'\\' || ch == L'/')
        m_config_file_path = cfg_dir + L"WeatherPro.ini";
    else
        m_config_file_path = cfg_dir + L"\\WeatherPro.ini";

    CSimpleIni ini;
    ini.SetUnicode();
    ini.LoadFile(m_config_file_path.c_str());

    m_currentCityInfo.CityNO = ini.GetValue(L"config", L"city_no", L"101010100");
    m_currentCityInfo.CityName = ini.GetValue(L"config", L"city_name", L"北京");

    m_config.m_api_type = static_cast<DataApiType>(ini.GetLongValue(L"config", L"api_type", 1));
    m_config.m_wit = static_cast<EWeatherInfoType>(ini.GetLongValue(L"config", L"wit", 0));
    m_config.m_update_frequency = static_cast<UpdateFrequency>(ini.GetLongValue(L"config", L"update_freq", 1));
    m_config.m_icon_type = static_cast<IconType>(ini.GetLongValue(L"config", L"icon_type", 0));
    m_config.m_show_weather_icon = ini.GetBoolValue(L"config", L"show_weather_icon", 1);
    m_config.m_show_weather_in_tooltips = ini.GetBoolValue(L"config", L"show_weather_in_tooltips", 1);
    m_config.m_show_brief_rt_weather_info = ini.GetBoolValue(L"config", L"show_brief_rt_weather_info", 0);
    m_config.m_show_weather_alerts = ini.GetBoolValue(L"config", L"show_weather_alerts", 1);
    m_config.m_show_brief_weather_alert_info = ini.GetBoolValue(L"config", L"show_brief_weather_alert_info", 1);
    m_config.m_show_error_info = ini.GetBoolValue(L"config", L"show_error_info", 0);
    m_config.m_double_click_action = ini.GetLongValue(L"config", L"double_click_action", 0);

    auto &hf_cfg = m_api_hfw->config;
    hf_cfg.AppKey = ini.GetValue(L"hfw", L"AppKey", L"");
    hf_cfg.ShowRealtimeTemperatureFeelsLike = ini.GetBoolValue(L"hfw", L"show_rt_temp_feels_like", 0);
    hf_cfg.ShowRealtimeWind = ini.GetBoolValue(L"hfw", L"show_rt_wind", 1);
    hf_cfg.ShowRealtimeWindScale = ini.GetBoolValue(L"hfw", L"show_rt_wind_scale", 1);
    hf_cfg.ShowRealtimeHumidity = ini.GetBoolValue(L"hfw", L"show_rt_humidity", 1);
    hf_cfg.ShowForecastUVIdex = ini.GetBoolValue(L"hfw", L"show_fc_uv_index", 0);
    hf_cfg.showForecastHumidity = ini.GetBoolValue(L"hfw", L"show_fc_humidity", 0);
    hf_cfg.ShowAirQuality = ini.GetBoolValue(L"hfw", L"show_rt_air_quality", 1);
    hf_cfg.ShowAirQualityAQI = ini.GetBoolValue(L"hfw", L"show_rt_air_quality_aqi", 1);
    hf_cfg.ShowAirQualityPM2p5 = ini.GetBoolValue(L"hfw", L"show_rt_air_quality_pm2p5", 1);
    hf_cfg.ShowAirQualityPM10 = ini.GetBoolValue(L"hfw", L"show_rt_air_quality_pm10", 0);
    hf_cfg.ShowWeatherAlert = ini.GetBoolValue(L"hfw", L"show_weather_alert", 1);
}

void CDataManager::SaveConfigs() const
{
    CSimpleIni ini;
    ini.SetUnicode();
    ini.LoadFile(m_config_file_path.c_str());

    ini.SetValue(L"config", L"city_no", m_currentCityInfo.CityNO.c_str());
    ini.SetValue(L"config", L"city_name", m_currentCityInfo.CityName.c_str());

    ini.SetLongValue(L"config", L"api_type", static_cast<int>(m_config.m_api_type));
    ini.SetLongValue(L"config", L"wit", static_cast<int>(m_config.m_wit));
    ini.SetLongValue(L"config", L"update_freq", static_cast<int>(m_config.m_update_frequency));
    ini.SetLongValue(L"config", L"icon_type", static_cast<int>(m_config.m_icon_type));
    ini.SetBoolValue(L"config", L"show_weather_icon", m_config.m_show_weather_icon);
    ini.SetBoolValue(L"config", L"show_weather_icon", m_config.m_show_weather_icon);
    ini.SetBoolValue(L"config", L"show_weather_in_tooltips", m_config.m_show_weather_in_tooltips);
    ini.SetBoolValue(L"config", L"show_brief_rt_weather_info", m_config.m_show_brief_rt_weather_info);
    ini.SetBoolValue(L"config", L"show_weather_alerts", m_config.m_show_weather_alerts);
    ini.SetBoolValue(L"config", L"show_brief_weather_alert_info", m_config.m_show_brief_weather_alert_info);
    ini.SetBoolValue(L"config", L"show_error_info", m_config.m_show_error_info);
    ini.SetLongValue(L"config", L"double_click_action", m_config.m_double_click_action);

    auto &hf_cfg = m_api_hfw->config;
    ini.SetValue(L"hfw", L"AppKey", hf_cfg.AppKey.c_str());
    ini.SetBoolValue(L"hfw", L"show_rt_temp_feels_like", hf_cfg.ShowRealtimeTemperatureFeelsLike);
    ini.SetBoolValue(L"hfw", L"show_rt_wind", hf_cfg.ShowRealtimeWind);
    ini.SetBoolValue(L"hfw", L"show_rt_wind_scale", hf_cfg.ShowRealtimeWindScale);
    ini.SetBoolValue(L"hfw", L"show_rt_humidity", hf_cfg.ShowRealtimeHumidity);
    ini.SetBoolValue(L"hfw", L"show_fc_uv_index", hf_cfg.ShowForecastUVIdex);
    ini.SetBoolValue(L"hfw", L"show_fc_humidity", hf_cfg.showForecastHumidity);
    ini.SetBoolValue(L"hfw", L"show_rt_air_quality", hf_cfg.ShowAirQuality);
    ini.SetBoolValue(L"hfw", L"show_rt_air_quality_aqi", hf_cfg.ShowAirQualityAQI);
    ini.SetBoolValue(L"hfw", L"show_rt_air_quality_pm2p5", hf_cfg.ShowAirQualityPM2p5);
    ini.SetBoolValue(L"hfw", L"show_rt_air_quality_pm10", hf_cfg.ShowAirQualityPM10);
    ini.SetBoolValue(L"hfw", L"show_weather_alert", hf_cfg.ShowWeatherAlert);

    ini.SaveFile(m_config_file_path.c_str());
}

HICON CDataManager::GetIcon()
{
    if (m_config.m_icon_type != m_weather_info_cache.CurrentIconType)
    {
        m_weather_info_cache.CurrentIconType = m_config.m_icon_type;
        m_weather_info_cache.Icon = _getIcon();
    }

    return m_weather_info_cache.Icon;
}

HICON CDataManager::_getIcon()
{
    auto api = GetCurrentApi();
    if (api != nullptr)
        return icon::get_icon(m_config.m_api_type, api->GetWeatherCode(m_config.m_wit), m_config.m_icon_type);
    else
        return nullptr;
}

std::wstring CDataManager::_getWeatherTemperature() const
{
    auto api = GetCurrentApi();
    if (api != nullptr)
    {
        if (m_config.m_show_weather_icon)
            return api->GetTemprature(m_config.m_wit);
        else
            return api->GetWeatherText(m_config.m_wit) + L" " + api->GetTemprature(m_config.m_wit);
    }
    else
        return L"";
}

std::wstring CDataManager::_getTooptipInfo() const
{
    auto api = GetCurrentApi();
    if (api != nullptr)
    {
        std::wstringstream wss;

        wss << m_currentCityInfo.CityName << L" " << api->GetWeatherInfoSummary();

        auto lastErr = api->GetLastError();
        if (m_config.m_show_error_info && !lastErr.empty())
        {
            wss << std::endl;

            wss << L"=====WeatherPro-Errors=====" << std::endl
                << lastErr << std::endl
                << L"===========================";
        }

        return wss.str();
    }

    return L"";
}

void CDataManager::RefreshWeatherInfoCache()
{
    m_weather_info_cache.WeatherTemperature = _getWeatherTemperature();
    m_weather_info_cache.TooltipInfo = _getTooptipInfo();
    m_weather_info_cache.Icon = _getIcon();
}
