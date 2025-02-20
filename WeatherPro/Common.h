#pragma once
#include "httplib.h"

using WStringList = std::vector<std::wstring>;

namespace utils
{
    // 转换宽窄字符串
    std::wstring multi_byte2wide_char(const char *str, bool is_utf8 = true);
    std::string wide_char2multi_byte(const wchar_t *str, bool is_utf_8 = true);

    int internet_get(const std::string &host, const std::string &path, std::wstring &content, WStringList &errors,
                     httplib::Headers headers = httplib::Headers());
}
