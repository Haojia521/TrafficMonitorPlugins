#pragma once

#include <string>

struct IpLocator
{
    std::wstring ip;
    std::wstring location;
    std::wstring longitude;
    std::wstring latitude;
    std::wstring err_message;

    virtual bool GetLocation() = 0;
};

struct IpLocatorIPIPNET : public IpLocator
{
    bool GetLocation() override;
};
