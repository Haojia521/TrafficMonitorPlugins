#pragma once

#include <string>

struct Locator
{
    std::wstring ip;
    std::wstring location_name;
    std::wstring longitude;
    std::wstring latitude;
    std::wstring err_message;

    virtual bool GetLocation() = 0;
};

struct IpLocatorIPIPNET : public Locator
{
    bool GetLocation() override;
};
