#include "stdafx.h"

#include "base64.hpp"


// base64 エンコード
bool algorithm::encode_base64(const std::vector<unsigned char>& src, std::string& dst)
{
    const std::string table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    std::string       cdst;

    for (std::size_t i = 0; i < src.size(); ++i) {
        switch (i % 3) {
        case 0:
            cdst.push_back(table[(src[i] & 0xFC) >> 2]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x03) << 4]);
                cdst.push_back('=');
                cdst.push_back('=');
            }

            break;
        case 1:
            cdst.push_back(table[((src[i - 1] & 0x03) << 4) | ((src[i + 0] & 0xF0) >> 4)]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x0F) << 2]);
                cdst.push_back('=');
            }

            break;
        case 2:
            cdst.push_back(table[((src[i - 1] & 0x0F) << 2) | ((src[i + 0] & 0xC0) >> 6)]);
            cdst.push_back(table[src[i] & 0x3F]);

            break;
        }
    }

    dst.swap(cdst);

    return true;
}


// Base64エンコード
bool algorithm::encode_base64(const std::vector<unsigned char>& src, std::vector<unsigned char>& dst)
{
    const std::string table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    std::vector<unsigned char> cdst;

    for (std::size_t i = 0; i < src.size(); ++i) {
        switch (i % 3) {
        case 0:
            cdst.push_back(table[(src[i] & 0xFC) >> 2]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x03) << 4]);
                cdst.push_back('=');
                cdst.push_back('=');
            }
            break;
        case 1:
            cdst.push_back(table[((src[i - 1] & 0x03) << 4) | ((src[i + 0] & 0xF0) >> 4)]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x0F) << 2]);
                cdst.push_back('=');
            }
            break;
        case 2:
            cdst.push_back(table[((src[i - 1] & 0x0F) << 2) | ((src[i + 0] & 0xC0) >> 6)]);
            cdst.push_back(table[src[i] & 0x3F]);
            break;
        }
    }
    dst.swap(cdst);
    return true;
}

bool algorithm::encode_base64w(const std::vector<unsigned char>& src, std::wstring& dst)
{
    const std::string table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    std::string       cdst;

    for (std::size_t i = 0; i < src.size(); ++i) {
        switch (i % 3) {
        case 0:
            cdst.push_back(table[(src[i] & 0xFC) >> 2]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x03) << 4]);
                cdst.push_back('=');
                cdst.push_back('=');
            }

            break;
        case 1:
            cdst.push_back(table[((src[i - 1] & 0x03) << 4) | ((src[i + 0] & 0xF0) >> 4)]);
            if (i + 1 == src.size()) {
                cdst.push_back(table[(src[i] & 0x0F) << 2]);
                cdst.push_back('=');
            }

            break;
        case 2:
            cdst.push_back(table[((src[i - 1] & 0x0F) << 2) | ((src[i + 0] & 0xC0) >> 6)]);
            cdst.push_back(table[src[i] & 0x3F]);

            break;
        }
    }

    // std::string を std::wstring に変換
    dst.clear();
    for (char c : cdst) {
        dst.push_back(static_cast<wchar_t>(c));
    }

    return true;
}

// base64 デコード
bool algorithm::decode_base64(const std::string& src, std::vector<unsigned char>& dst)
{
    if (src.size() & 0x00000003) {
        return false;
    }
    else {
        const std::string          table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
        std::vector<unsigned char> cdst;

        for (std::size_t i = 0; i < src.size(); i += 4) {
            if (src[i + 0] == '=') {
                return false;
            }
            else if (src[i + 1] == '=') {
                return false;
            }
            else if (src[i + 2] == '=') {
                const std::string::size_type s1 = table.find(src[i + 0]);
                const std::string::size_type s2 = table.find(src[i + 1]);

                if (s1 == std::string::npos || s2 == std::string::npos) {
                    return false;
                }

                cdst.push_back(static_cast<unsigned char>(((s1 & 0x3F) << 2) | ((s2 & 0x30) >> 4)));

                break;
            }
            else if (src[i + 3] == '=') {
                const std::string::size_type s1 = table.find(src[i + 0]);
                const std::string::size_type s2 = table.find(src[i + 1]);
                const std::string::size_type s3 = table.find(src[i + 2]);

                if (s1 == std::string::npos || s2 == std::string::npos || s3 == std::string::npos) {
                    return false;
                }

                cdst.push_back(static_cast<unsigned char>(((s1 & 0x3F) << 2) | ((s2 & 0x30) >> 4)));
                cdst.push_back(static_cast<unsigned char>(((s2 & 0x0F) << 4) | ((s3 & 0x3C) >> 2)));

                break;
            }
            else {
                const std::string::size_type s1 = table.find(src[i + 0]);
                const std::string::size_type s2 = table.find(src[i + 1]);
                const std::string::size_type s3 = table.find(src[i + 2]);
                const std::string::size_type s4 = table.find(src[i + 3]);

                if (s1 == std::string::npos || s2 == std::string::npos || s3 == std::string::npos || s4 == std::string::npos) {
                    return false;
                }

                cdst.push_back(static_cast<unsigned char>(((s1 & 0x3F) << 2) | ((s2 & 0x30) >> 4)));
                cdst.push_back(static_cast<unsigned char>(((s2 & 0x0F) << 4) | ((s3 & 0x3C) >> 2)));
                cdst.push_back(static_cast<unsigned char>(((s3 & 0x03) << 6) | ((s4 & 0x3F) >> 0)));
            }
        }

        dst.swap(cdst);

        return true;
    }
}

void saveImageAsHtml(const std::string& image_base64, const std::string& filename) 
{
    std::ofstream htmlFile(filename);
    if (htmlFile.is_open()) {
        htmlFile << "<!DOCTYPE html>\n";
        htmlFile << "<html lang=\"en\">\n";
        htmlFile << "<head>\n";
        htmlFile << "    <meta charset=\"UTF-8\">\n";
        htmlFile << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        htmlFile << "    <title>Base64 Image Test</title>\n";
        htmlFile << "</head>\n";
        htmlFile << "<body>\n";
        htmlFile << "    <h1>Embedded Image</h1>\n";
        htmlFile << "    <p>This image is embedded directly from a Base64 string:</p>\n";
        htmlFile << "    <img src=\"data:image/jpeg;base64," << image_base64 << "\" alt=\"Embedded Image\">\n";
        htmlFile << "</body>\n";
        htmlFile << "</html>\n";
        htmlFile.close();
    }
    else {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
    }
}
void saveImageAsHtmlw(const std::wstring& image_base64, const std::string& filename)
{
    std::wofstream htmlFile(filename);
    if (htmlFile.is_open()) {
        htmlFile << L"<!DOCTYPE html>\n";
        htmlFile << L"<html lang=\"en\">\n";
        htmlFile << L"<head>\n";
        htmlFile << L"    <meta charset=\"UTF-8\">\n";
        htmlFile << L"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        htmlFile << L"    <title>Base64 Image Test</title>\n";
        htmlFile << L"</head>\n";
        htmlFile << L"<body>\n";
        htmlFile << L"    <h1>Embedded Image</h1>\n";
        htmlFile << L"    <p>This image is embedded directly from a Base64 string:</p>\n";
        htmlFile << L"    <img src=\"data:image/jpeg;base64," << image_base64 << L"\" alt=\"Embedded Image\">\n";
        htmlFile << L"</body>\n";
        htmlFile << L"</html>\n";
        htmlFile.close();
    }
    else {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
    }
}