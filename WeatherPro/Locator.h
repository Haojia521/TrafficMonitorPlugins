#pragma once

#include <string>

struct LocationData
{
    std::wstring ip;
    std::wstring location_name;
    std::wstring longitude;
    std::wstring latitude;
    std::wstring err_message;
};

class Locator
{
public:
    virtual bool GetLocation(LocationData &loc) = 0;
};

class IpLocatorIPIPNET: public Locator
{
public:
    bool GetLocation(LocationData &loc) override;
};
