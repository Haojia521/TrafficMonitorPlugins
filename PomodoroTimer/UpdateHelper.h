#pragma once

#include <string>
#include <vector>

enum class EUpdateSource
{
    EC_GITHUB,
    EC_GITEE,
};

using NumberVersion = std::vector<int>;

struct EVersionInfo
{
    std::wstring m_str_version;
    NumberVersion m_num_version;

    EUpdateSource m_source;

    std::wstring m_link_x86;
    std::wstring m_link_x64;
};

class CUpdateHelper
{
    CUpdateHelper() = default;
public:
    static CUpdateHelper& Instance();

    bool HasNewVersion() const;

    void CheckNewVersion();

    std::wstring GetReleaseLink() const;

private:
    static CUpdateHelper m_instance;

    NumberVersion m_current_version;
    EVersionInfo m_newest_release;
};
