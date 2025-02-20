#include "pch.h"
#include "Common.h"
#include <memory>
#include <vector>
#include <format>

namespace utils
{
    std::wstring multi_byte2wide_char(const char *str, bool is_utf8 /*= true*/) {
        if (str == nullptr) return std::wstring();

        auto code_page = is_utf8 ? CP_UTF8 : CP_ACP;
        auto len = MultiByteToWideChar(code_page, 0, str, -1, nullptr, 0);
        if (len <= 0) return std::wstring();

        std::vector<wchar_t> buffer(len, 0);
        MultiByteToWideChar(code_page, 0, str, -1, buffer.data(), len);

        return std::wstring(buffer.data());
    }

    std::string wide_char2multi_byte(const wchar_t *str, bool is_utf8 /*= true*/) {
        if (str == nullptr) return std::string();

        auto code_page = is_utf8 ? CP_UTF8 : CP_ACP;
        auto len = WideCharToMultiByte(code_page, 0, str, -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) return std::string();

        std::vector<char> buffer(len, 0);
        WideCharToMultiByte(code_page, 0, str, -1, buffer.data(), len, nullptr, nullptr);

        return std::string(buffer.data());
    }

    int internet_get(const std::string &host, const std::string &path, std::wstring &content, WStringList &errors,
                     httplib::Headers headers /*= httplib::Headers()*/)
    {
        content.clear();

        httplib::Client clt(host);

        auto res = headers.empty() ? clt.Get(path) : clt.Get(path, headers);
        if (res) {
            if (res->status == 404) return res->status;

            content = multi_byte2wide_char(res->body.c_str());
            return res->status;
        } else {
            errors.push_back(std::format(L"ResponseError: {}", multi_byte2wide_char(httplib::to_string(res.error()).c_str())));
            return 0;
        }
    }
}
