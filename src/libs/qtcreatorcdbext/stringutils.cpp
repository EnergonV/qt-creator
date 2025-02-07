// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#ifndef _SCL_SECURE_NO_WARNINGS // silence std::string::copy
# define _SCL_SECURE_NO_WARNINGS
#endif

#include "stringutils.h"

#include <cctype>
#include <cstring>
#include <iostream>
#include <sstream>
#include <codecvt>
#include <iomanip>

#include <windows.h>

static const char whiteSpace[] = " \t\r\n";

void trimFront(std::string &s)
{
    if (s.empty())
        return;
    std::string::size_type pos = s.find_first_not_of(whiteSpace);
    if (pos == 0)
        return;
    if (pos == std::string::npos) { // All blanks?!
        s.clear();
    } else {
        s.erase(0, pos);
    }
}

void trimBack(std::string &s)
{
    if (s.empty())
        return;
    std::string::size_type pos = s.find_last_not_of(whiteSpace);
    if (pos == std::string::npos) { // All blanks?!
        s.clear();
    } else {
        if (++pos != s.size())
            s.erase(pos, s.size() - pos);
    }
}

void replace(std::wstring &s, wchar_t before, wchar_t after)
{
    const std::wstring::size_type size = s.size();
    for (std::wstring::size_type i = 0; i < size; ++i)
        if (s.at(i) == before)
            s[i] = after;
}

void replace(std::string &s, char before, char after)
{
    const std::string::size_type size = s.size();
    for (std::string::size_type i = 0; i < size; ++i)
        if (s.at(i) == before)
            s[i] = after;
}

bool endsWith(const std::string &haystack, const char *needle)
{
    const size_t needleLen = strlen(needle);
    const size_t haystackLen = haystack.size();
    if (needleLen > haystackLen)
        return false;
    return haystack.compare(haystackLen - needleLen, needleLen, needle) == 0;
}

static inline void formatGdbmiChar(std::ostream &str, wchar_t c)
{
    switch (c) {
    case L'\n':
        str << "\\n";
        break;
    case L'\t':
        str << "\\t";
        break;
    case L'\r':
        str << "\\r";
        break;
    case L'\\':
    case L'"':
        str << '\\' << char(c);
        break;
    default:
        if (c < 128) {
            str << char(c);
        } else {
            // Always pad up to 3 digits in case a digit follows
            const char oldFill = str.fill('0');
            str << '\\' << std::oct;
            str.width(3);
            str << unsigned(c) << std::dec;
            str.fill(oldFill);
        }
        break;
    }
}

// Stream  a wstring onto a char stream doing octal escaping
// suitable for GDBMI.

void gdbmiStringFormat::format(std::ostream &str) const
{
    const std::string::size_type size = m_s.size();
    for (std::string::size_type i = 0; i < size; ++i)
        formatGdbmiChar(str, wchar_t(m_s.at(i)));
}

void gdbmiWStringFormat::format(std::ostream &str) const
{
    const std::wstring::size_type size = m_w.size();
    for (std::wstring::size_type i = 0; i < size; ++i)
        formatGdbmiChar(str, m_w.at(i));
}

std::string wStringToString(const std::wstring &w)
{
    if (w.empty())
        return std::string();
    const std::string::size_type size = w.size();
    std::string rc;
    rc.reserve(size);
    for (std::string::size_type i = 0; i < size; ++i)
        rc.push_back(char(w.at(i)));
    return rc;
}

std::wstring utf8ToUtf16(const std::string &s)
{
    const int size = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), result.data(), size);
    return result;
}

// Convert an ASCII hex digit to its value 'A'->10
static inline unsigned hexDigit(char c)
{
    if (c <= '9')
        return c - '0';
    if (c <= 'F')
        return c - 'A' + 10;
    return c - 'a' + 10;
}

// Convert an ASCII hex digit to its value 'A'->10
static char toHexDigit(unsigned char v)
{
    return v < 10 ? char(v) + '0' : char(v) - 10 + 'a';
}

// Strings from raw data.
std::wstring quotedWStringFromCharData(const unsigned char *data, size_t size, bool truncated)
{
    std::wstring rc;
    rc.reserve(size + (truncated ? 5 : 2));
    rc.push_back(L'"');
    const unsigned char *end = data + size;
    for ( ; data < end; data++)
        rc.push_back(wchar_t(*data));
    if (truncated)
        rc.append(L"...");
    rc.push_back(L'"');
    return rc;
}

std::wstring quotedWStringFromWCharData(const unsigned char *dataIn, size_t sizeIn, bool truncated)
{
    std::wstring rc;
    const wchar_t *data = reinterpret_cast<const wchar_t *>(dataIn);
    const size_t size = sizeIn / sizeof(wchar_t);
    rc.reserve(size + (truncated ? 5 : 2));
    rc.push_back(L'"');
    rc.append(data, data + size);
    if (truncated)
        rc.append(L"...");
    rc.push_back(L'"');
    return rc;
}

// String from hex "414A" -> "AJ".
std::string stringFromHex(const char *p, const char *end)
{
    if (p == end)
        return std::string();

    std::string rc;
    rc.reserve((end - p) / 2);
    for ( ; p < end; ++p) {
        unsigned c = 16 * hexDigit(*p);
        c += hexDigit(*++p);
        rc.push_back(char(c));
    }
    return rc;
}

std::string stringFromHex(const std::string &hexEncoded)
{
    return stringFromHex(hexEncoded.c_str(), hexEncoded.c_str() + hexEncoded.size());
}

// Helper for dumping memory
std::string dumpMemory(const unsigned char *p, size_t size,
                       bool wantQuotes)
{
    std::ostringstream str;
    str << std::oct << std::setfill('0');
    if (wantQuotes)
        str << '"';
    const unsigned char *end = p + size;
    for ( ; p < end; ++p) {
        const unsigned char u = *p;
        switch (u) {
        case '\t':
            str << "\\t";
            break;
        case '\r':
            str << "\\r";
            break;
        case '\n':
            str << "\\n";
            break;
        default:
            if (u >= 32 && u < 128)
                str << (char(u));
            else
                str  << '\\' << std::setw(3) << unsigned(u);
            break;
        }
    }
    if (wantQuotes)
        str << '"';
    return str.str();
}

void decodeHex(const char *p, const char *end, unsigned char *target)
{
    for ( ; p < end; ++p) {
        unsigned c = 16 * hexDigit(*p);
        c += hexDigit(*++p);
        *target++ = c;
    }
}

MemoryHandle *MemoryHandle::fromStdString(const std::string &s)
{
    const size_t size = s.size();
    unsigned char *data = new unsigned char[size];
    s.copy(reinterpret_cast<char *>(data), size);
    return new MemoryHandle(data, size);
}

MemoryHandle *MemoryHandle::fromStdWString(const std::wstring &ws)
{
    const size_t size = ws.size();
    wchar_t *data = new wchar_t[size];
    ws.copy(data, size);
    return new MemoryHandle(data, size);
}

template <class String>
inline String dataToHexHelper(const unsigned char *p, const unsigned char *end)
{
    if (p == end)
        return String();

    String rc;
    rc.reserve(2 * (end - p));
    for ( ; p < end ; ++p) {
        const unsigned char c = *p;
        rc.push_back(toHexDigit(c / 16));
        rc.push_back(toHexDigit(c &0xF));
    }
    return rc;
}

std::wstring dataToHexW(const unsigned char *p, const unsigned char *end)
{
    return dataToHexHelper<std::wstring>(p, end);
}

std::string dataToHex(const unsigned char *p, const unsigned char *end)
{
    return dataToHexHelper<std::string>(p, end);
}

// Readable hex: '0xAA 0xBB'..
std::wstring dataToReadableHexW(const unsigned char *begin, const unsigned char *end)
{
    if (begin == end)
        return std::wstring();

    std::wstring rc;
    rc.reserve(5 * (end - begin));
    for (const unsigned char *p = begin; p < end ; ++p) {
        rc.append(p == begin ? L"0x" : L" 0x");
        const unsigned c = *p;
        rc.push_back(toHexDigit(c / 16));
        rc.push_back(toHexDigit(c &0xF));
    }
    return rc;
}

// Format a map as a GDBMI hash {key="value",..}
void formatGdbmiHash(std::ostream &os, const std::map<std::string, std::string> &m, bool closeHash)
{
    typedef std::map<std::string, std::string>::const_iterator It;
    const It begin = m.begin();
    const It cend = m.end();
    os << '{';
    for (It it = begin; it != cend; ++it) {
        if (it != begin)
            os << ',';
        os << it->first << "=\"" << gdbmiStringFormat(it->second) << '"';
    }
    if (closeHash)
        os << '}';
}

void hexEncode(std::ostream &str, const unsigned char *source, size_t sourcelen)
{
    for (size_t i = 0; i < sourcelen; ++i)
        str << toHexDigit(source[i] >> 4) << toHexDigit(source[i] & 0xf);
}
