﻿#pragma once
#include <string>
#include <zlib.h>
class CCommon
{
public:
    //将const char*字符串转换成宽字符字符串
    static std::wstring StrToUnicode(const char* str, bool utf8 = false);

    static std::string UnicodeToStr(const wchar_t* wstr, bool utf8 = false);

    //将一个字符串转换成URL编码（以UTF8编码格式）
    static std::wstring URLEncode(const std::wstring& wstr);

    // 解码gzip压缩的字节数据
    static int GZipDecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata);
};
