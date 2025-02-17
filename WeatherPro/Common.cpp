#include "pch.h"
#include "Common.h"
#include <afxinet.h>    //用于支持使用网络相关的类
#include <memory>
#include <vector>
#include <format>

std::wstring CCommon::StrToUnicode(const char* str, bool utf8)
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

std::string CCommon::UnicodeToStr(const wchar_t* wstr, bool utf8)
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

std::wstring CCommon::URLEncode(const std::wstring& wstr)
{
    std::string str_utf8;
    std::wstring result{};
    wchar_t buff[4];
    str_utf8 = CCommon::UnicodeToStr(wstr.c_str(), true);
    for (const auto& ch : str_utf8)
    {
        if (ch == ' ')
            result.push_back(L'+');
        else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
            result.push_back(static_cast<wchar_t>(ch));
        else if (ch == '-' || ch == '_' || ch == '.' || ch == '!' || ch == '~' || ch == '*'/* || ch == '\''*/ || ch == '(' || ch == ')')
            result.push_back(static_cast<wchar_t>(ch));
        else
        {
            swprintf_s(buff, L"%%%x", static_cast<unsigned char>(ch));
            result += buff;
        }
    }
    return result;
}

int CCommon::GZipDecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata)
{
    int err = 0;
    z_stream d_stream = { 0 }; /* decompression stream */
    unsigned char dummy_head[2] = { 0x1F, 0x8B };
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;

    if (inflateInit2(&d_stream, 47) != Z_OK) return -1;
    while (d_stream.total_in < nzdata) {
        d_stream.avail_in = d_stream.avail_out = 2048; /* force small buffers */
        if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
        if (err != Z_OK)
        {
            if (err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*)dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                {
                    return -1;
                }
            }
            else return -1;
        }
    }
    *ndata = d_stream.total_out;
    if (inflateEnd(&d_stream) != Z_OK) return -1;

    return 0;
}

unsigned long CCommon::AccessInternet(const std::wstring &url, std::wstring &content, WStringList &errors,
                                      const InternetConfig &cfg /* = InternetConfig() */)
{
    content.clear();

    DWORD dw_status_code{ 0 };
    UINT buffer_len{ 0 };
    std::vector<char> http_content_buffer(1024 * 8, 0);
    try
    {
        std::unique_ptr<CInternetSession, void(*)(CInternetSession*)> session(
            new CInternetSession(cfg.agent.c_str()),
            [](CInternetSession *p) { if (p != nullptr) p->Close(); delete p; }
        );

        std::unique_ptr<CHttpFile, void(*)(CHttpFile*)> http_file(
            dynamic_cast<CHttpFile*>(session->OpenURL(url.c_str(), 1, INTERNET_FLAG_TRANSFER_ASCII, cfg.headers.c_str())),
            [](CHttpFile *p) { if (p != nullptr) p->Close(); delete p; }
        );

        http_file->QueryInfoStatusCode(dw_status_code);

        if (dw_status_code == HTTP_STATUS_NOT_FOUND)
            return HTTP_STATUS_NOT_FOUND;

        buffer_len = http_file->Read(http_content_buffer.data(), 1024 * 8);
    }
    catch (CInternetException *e)
    {
        TCHAR cause[1024]{ 0 };
        if (e->GetErrorMessage(cause, 1024) == TRUE)
            errors.push_back(std::format(L"[InternetException {}] {}", e->m_dwError, cause));
        e->Delete();
    }

    if (cfg.gzip)
    {
        std::vector<Byte> decomp_contents_buffer(1024 * 64, 0);
        uLong decomp_contents_len{ 0 };
        if (CCommon::GZipDecompress((Byte*)http_content_buffer.data(), (uLong)buffer_len,
                                    decomp_contents_buffer.data(), &decomp_contents_len) == 0) {
            content = CCommon::StrToUnicode((char*)decomp_contents_buffer.data(), true);
        } else
            errors.push_back(L"failed to decompress http contents via gzip");
    } else
    {
        content = CCommon::StrToUnicode(http_content_buffer.data(), true);
    }

    return dw_status_code;
}
