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

    /**
     * @brief 生成 ED25519 密钥对
     * @param errors 输出参数，用于存储错误信息
     * @return 包含 (private_key, public_key) 的元组，失败时均为空字符串
     */
    std::tuple<std::string, std::string> generate_ed25519_keypair(std::vector<std::string>& errors) noexcept;
}
