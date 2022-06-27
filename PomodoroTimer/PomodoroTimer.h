#pragma once

#include <PluginInterface.h>
#include "Item.h"

class CPomodoroTimer : public ITMPlugin
{
private:
    CPomodoroTimer();

public:
    static CPomodoroTimer& Instance();

    IPluginItem* GetItem(int index) override;
    const wchar_t* GetTooltipInfo() override;
    void DataRequired() override;
    OptionReturn ShowOptionsDialog(void* hParent) override;
    const wchar_t* GetInfo(PluginInfoIndex index) override;
    void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;

private:
    static CPomodoroTimer m_instance;
    CPtItem m_item;
};

#ifdef __cplusplus
extern "C" {
#endif // DEBUG
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif
