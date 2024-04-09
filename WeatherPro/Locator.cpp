#include "pch.h"
#include "Locator.h"
#include "Common.h"

#include <memory>
#include <afxinet.h>
#include <utilities/yyjson/yyjson.h>

namespace loc
{
    static bool call_internet(const CString &url, std::wstring &content, std::wstring &err)
    {
        content.clear();
        err.clear();

        bool succeed = false;

        unsigned char *contents_buffer = new unsigned char[1024 * 8] { 0 };  // 假设内容长度不会超过8k字节
        unsigned long contents_len{ 0 };

        try
        {
            std::unique_ptr<CInternetSession, void(*)(CInternetSession*)> session(
                new CInternetSession(),
                [](CInternetSession *p) {
                    if (p != nullptr) p->Close();
                    delete p;
                }
            );

            std::unique_ptr<CHttpFile, void(*)(CHttpFile*)> http_file(
                dynamic_cast<CHttpFile*>(session->OpenURL(url)),
                [](CHttpFile *p) {
                    if (p != nullptr) p->Close();
                    delete p;
                }
            );

            DWORD dw_status_code{ 0 };
            http_file->QueryInfoStatusCode(dw_status_code);

            if (dw_status_code == HTTP_STATUS_OK)
            {
                auto http_content_len = http_file->Seek(0, CFile::end);
                std::unique_ptr<char[]> http_content_buf(new char[http_content_len + 1] {0});

                http_file->Seek(0, CFile::begin);
                http_file->Read(http_content_buf.get(), static_cast<UINT>(http_content_len + 1));

                content = CCommon::StrToUnicode(http_content_buf.get(), true);
                succeed = true;
            }
            else
            {
                err = std::wstring(L"HTTP CODE: ") + std::to_wstring(dw_status_code);
            }
        }
        catch (CInternetException *e)
        {
            TCHAR cause[1024]{ 0 };
            if (e->GetErrorMessage(cause, 1024) == TRUE)
                err = cause;
            e->Delete();
        }

        return succeed;
    }

    static std::string get_json_str_value(yyjson_val *j_val, const char *key)
    {
        auto *obj = yyjson_obj_get(j_val, key);
        if (obj == nullptr) return "";
        else return yyjson_get_str(obj);
    }
}

bool IpLocatorIPIPNET::GetLocation()
{
    CString url(L"https://myip.ipip.net/json");

    std::wstring content;

    if (loc::call_internet(url, content, this->err_message))
    {
        auto json = CCommon::UnicodeToStr(content.c_str(), true);
        std::unique_ptr<yyjson_doc, void(*)(yyjson_doc*)> json_doc(
            yyjson_read(json.c_str(), json.size(), 0),
            [](yyjson_doc *p) {
                yyjson_doc_free(p);
            }
        );

        if (json_doc != nullptr)
        {
            auto *root = yyjson_doc_get_root(json_doc.get());

            if (loc::get_json_str_value(root, "ret") == "ok")
            {
                auto *data = yyjson_obj_get(root, "data");

                auto ip = loc::get_json_str_value(data, "ip");
                this->ip = CCommon::StrToUnicode(ip.c_str(), true);

                auto *location = yyjson_obj_get(data, "location");
                std::string prov = yyjson_get_str(yyjson_arr_get(location, 1));
                std::string city = yyjson_get_str(yyjson_arr_get(location, 2));

                this->location = CCommon::StrToUnicode((prov + city).c_str(), true);

                return true;
            }
            else
                this->err_message = L"[ipip.net] invalid return statue";
        }
        else
            this->err_message = L"[ipip.net] contents cannot be parsed as json";
    }

    return false;
}
