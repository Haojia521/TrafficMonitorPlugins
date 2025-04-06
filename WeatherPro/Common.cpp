#include "pch.h"
#include "Common.h"
#include <memory>
#include <vector>
#include <format>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

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

    // RAII 包装器
    struct EvpPkeyDeleter { void operator()(EVP_PKEY* p) noexcept { EVP_PKEY_free(p); } };
    using EvpPkeyPtr = std::unique_ptr<EVP_PKEY, EvpPkeyDeleter>;

    struct BioDeleter { void operator()(BIO* p) noexcept { BIO_free(p); } };
    using BioPtr = std::unique_ptr<BIO, BioDeleter>;

    struct PkeyCtxDeleter { void operator()(EVP_PKEY_CTX* p) noexcept { EVP_PKEY_CTX_free(p); } };
    using PkeyCtxPtr = std::unique_ptr<EVP_PKEY_CTX, PkeyCtxDeleter>;

    std::tuple<std::string, std::string> generate_ed25519_keypair(std::vector<std::string>& errors) noexcept {
        errors.clear();

        // 初始化 OpenSSL 上下文
        PkeyCtxPtr ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr));
        if (!ctx) {
            errors.emplace_back("Failed to create EVP_PKEY_CTX");
            return { "", "" };
        }

        // 密钥生成初始化
        if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
            errors.emplace_back("Failed to initialize key generation");
            return { "", "" };
        }

        // 生成密钥
        EVP_PKEY* raw_pkey = nullptr;
        if (EVP_PKEY_keygen(ctx.get(), &raw_pkey) <= 0) {
            errors.emplace_back("Failed to generate key pair");
            return { "", "" };
        }
        EvpPkeyPtr pkey(raw_pkey);

        // 将私钥写入内存 BIO
        auto export_to_pem = [&errors](const EVP_PKEY* key, std::function<int(BIO*, const EVP_PKEY*)> write_func) -> std::string {
            BioPtr bio(BIO_new(BIO_s_mem()));
            if (!bio) {
                errors.emplace_back("Failed to create BIO");
                return "";
            }

            if (write_func(bio.get(), key) <= 0) {
                errors.emplace_back("Failed to write key to BIO");
                return "";
            }

            char* data = nullptr;
            long len = BIO_get_mem_data(bio.get(), &data);
            return std::string(data, static_cast<size_t>(len));
        };

        // 导出公私钥
        std::string priv_pem = export_to_pem(pkey.get(),
                                             [](BIO *bio, const EVP_PKEY *k) -> int {
                                                 return PEM_write_bio_PrivateKey(bio, k, nullptr, nullptr, 0, nullptr, nullptr);
                                             }
        );
        std::string pub_pem = export_to_pem(pkey.get(), PEM_write_bio_PUBKEY);

        return { std::move(priv_pem), std::move(pub_pem) };
    }
}
