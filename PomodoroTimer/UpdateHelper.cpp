#include "pch.h"
#include "UpdateHelper.h"

#include <afxinet.h>

namespace hp
{
    std::wstring StrToUnicode(const char* str, bool utf8)
    {
        if (str == nullptr)
            return std::wstring();
        std::wstring result;
        int size;
        size = MultiByteToWideChar((utf8 ? CP_UTF8 : CP_ACP), 0, str, -1, NULL, 0);
        if (size <= 0) return std::wstring();
        wchar_t* str_unicode = new wchar_t[size + 1];
        MultiByteToWideChar((utf8 ? CP_UTF8 : CP_ACP), 0, str, -1, str_unicode, size);
        result.assign(str_unicode);
        delete[] str_unicode;
        return result;
    }

    std::string UnicodeToStr(const wchar_t* wstr, bool utf8)
    {
        if (wstr == nullptr)
            return std::string();
        std::string result;
        int size{ 0 };
        size = WideCharToMultiByte((utf8 ? CP_UTF8 : CP_ACP), 0, wstr, -1, NULL, 0, NULL, NULL);
        if (size <= 0) return std::string();
        char* str = new char[size + 1];
        WideCharToMultiByte((utf8 ? CP_UTF8 : CP_ACP), 0, wstr, -1, str, size, NULL, NULL);
        result.assign(str);
        delete[] str;
        return result;
    }

    class UrlGuard
    {
    public:
        ~UrlGuard()
        {
            if (m_session != nullptr)
            {
                m_session->Close();
                delete m_session;
            }

            if (m_file != nullptr)
            {
                m_file->Close();
                delete m_file;
            }
        }

        CInternetSession *m_session{ nullptr };
        CHttpFile *m_file{ nullptr };
    };

    bool CallUrl(const std::wstring& url, std::string& result, bool utf8)
    {
        result.clear();

        bool succeed = false;

        CInternetSession *session{ nullptr };
        CHttpFile *httpFile{ nullptr };

        char *multi_byte_content_buffer{ nullptr };

        try
        {
            session = new CInternetSession();
            httpFile = (CHttpFile *)session->OpenURL(url.c_str());
            DWORD dwStatusCode;
            httpFile->QueryInfoStatusCode(dwStatusCode);
            if (dwStatusCode == HTTP_STATUS_OK)
            {
                auto offset = httpFile->Seek(0, CFile::end);
                multi_byte_content_buffer = new char[offset + 1]{ 0 };

                httpFile->Seek(0, CFile::begin);
                httpFile->Read(multi_byte_content_buffer, static_cast<UINT>(offset + 1));

                result = multi_byte_content_buffer;
                succeed = true;
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

        delete[] multi_byte_content_buffer;
        multi_byte_content_buffer = nullptr;

        delete httpFile;
        httpFile = nullptr;

        delete session;
        session = nullptr;

        return succeed;
    }

    std::wstring github_source_link{ L"https://raw.githubusercontent.com/Haojia521/TrafficMonitorPlugins/dev-pomodoro-timer/version_pt.json" };
    std::wstring gitee_source_link{ L"https://gitee.com/Haojia521/TrafficMonitorPlugins/raw/dev-pomodoro-timer/version_pt.json" };

    bool GetVersionInfoByGithub(EVersionInfo &info)
    {
        // todo
    }

    bool GetVersionInfoByGitee(EVersionInfo &info)
    {
        // todo
    }
}

CUpdateHelper CUpdateHelper::m_instance;

CUpdateHelper& CUpdateHelper::Instance()
{
    return m_instance;
}

bool CUpdateHelper::HasNewVersion() const
{
    return m_newest_release.m_num_version > m_current_version;
}

void CUpdateHelper::CheckNewVersion()
{
    // check new version using the source of Gitee first
    if (!hp::GetVersionInfoByGitee(m_newest_release))
        hp::GetVersionInfoByGithub(m_newest_release);
}

std::wstring CUpdateHelper::GetReleaseLink() const
{
#ifdef _M_X64
    return m_newest_release.m_link_x64;
#else
    return m_newest_release.m_link_x86;
#endif // _M_X64
}
