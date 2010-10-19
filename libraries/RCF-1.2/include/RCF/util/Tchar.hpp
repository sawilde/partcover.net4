
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_TCHAR_HPP
#define INCLUDE_UTIL_TCHAR_HPP

#include <string>
#include <vector>

namespace util {

#ifndef BOOST_NO_STD_WSTRING

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )  // warning C4996: 'ctime' was declared deprecated
#endif

    inline std::wstring stringToWstring(const std::string &s)
    {
        std::wstring ws;
        if (!s.empty())
        {
            const char *sz = s.c_str();
            std::size_t szlen = s.length();
            std::vector<wchar_t> vec(szlen);
            wchar_t *wsz = &vec[0];
            std::size_t wszlen = mbstowcs(wsz, sz, szlen);
            if (wszlen == std::size_t(-1)) throw std::runtime_error("mbstowcs() failed");
            ws.assign(wsz, wszlen);
        }
        return ws;
    }

    inline std::string wstringToString(const std::wstring &ws)
    {
        std::string s;
        if (!ws.empty())
        {
            const wchar_t *wsz = ws.c_str();
            std::size_t wszlen = ws.length();
            std::vector<char> vec(4*wszlen);
            char *sz = &vec[0];
            std::size_t szlen = wcstombs(sz, wsz, wszlen);
            if (szlen == std::size_t(-1)) throw std::runtime_error("wcstombs() failed");
            s.assign(sz, szlen);
        }
        return s;
    }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

#if defined(BOOST_NO_STD_WSTRING)

    #ifdef UNICODE
    #error RCF does not currently support Unicode builds without wstring support
    #endif

    #define RCF_T(x)                            x
    typedef std::string                         tstring;
    inline tstring toTstring(std::string s)     { return s; }
    inline std::string toString(tstring s)      { return s; }


#elif defined(UNICODE)

    #define RCF_T(x)                            L ## x                        
    typedef std::wstring                        tstring;
    inline tstring toTstring(std::string s)     { return stringToWstring(s); }
    inline tstring toTstring(std::wstring s)    { return s; }
    inline std::string toString(tstring s)      { return wstringToString(s); }
    inline std::wstring toWstring(tstring s)    { return s; }

#else

    #define RCF_T(x)                            x
    typedef std::string                         tstring;
    inline tstring toTstring(std::string s)     { return s; }
    inline tstring toTstring(std::wstring ws)   { return wstringToString(ws); }
    inline std::string toString(tstring s)      { return s; }
    inline std::wstring toWstring(tstring s)    { return stringToWstring(s); }

#endif

} // namespace util

#endif // ! INCLUDE_UTIL_TCHAR_HPP
