#pragma once

#include <include/PluginInterface.h>

enum class PermanmentItemType
{
    RT_Humidity = 1 << 0,
    RT_Wind     = 1 << 1,
    RT_AQI      = 1 << 2,
    RT_PM2P5    = 1 << 3,
    RT_PM10     = 1 << 4,
};

class CPermanmentItem : public IPluginItem
{
public:
    CPermanmentItem(PermanmentItemType t);

    const wchar_t* GetItemName() const override;
    const wchar_t* GetItemId() const override;
    const wchar_t* GetItemLableText() const override;
    const wchar_t* GetItemValueText() const override;
    const wchar_t* GetItemValueSampleText() const override;

private:
    PermanmentItemType type;
};
