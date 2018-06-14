#ifndef __ANDROID__
#include <iconv.h>
#endif
#include <stdlib.h>
#include <memory.h>

#include "charset.h"

namespace shove
{
namespace encode
{

string Charset::ws2s(const wstring& ws)
{
    string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
#ifdef _WIN32
    setlocale(LC_ALL, "chs");
#else
    setlocale(LC_ALL, "zh_CN.UTF-8");
#endif

    const wchar_t* _source = ws.c_str();
    size_t _dsize = 2 * ws.size() + 1;
    char * _dest = new char[_dsize];
    memset(_dest, 0 ,_dsize);
    wcstombs(_dest, _source, _dsize);
    string result = _dest;

    if (_dest != NULL)
    {
        delete[] _dest;
        _dest = NULL;
    }

    //setlocale(LC_ALL, "C");
    setlocale(LC_ALL, curLocale.c_str());

    return result;
}

wstring Charset::s2ws(const string& s)
{
    string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
#ifdef _WIN32
    setlocale(LC_ALL, "chs");
#else
    setlocale(LC_ALL, "zh_CN.UTF-8");
#endif

    const char* _source = s.c_str();
    size_t _dsize = s.size() + 1;
    wchar_t *_dest = new wchar_t[_dsize];
    wmemset(_dest, 0, _dsize);
    mbstowcs(_dest, _source, _dsize);
    wstring result = _dest;

    if (_dest != NULL)
    {
        delete[] _dest;
        _dest = NULL;
    }

    //setlocale(LC_ALL, "C");
    setlocale(LC_ALL, curLocale.c_str());

    return result;
}

wstring Charset::utf2uni(const char* src, wstring& t)
{
    if (src == NULL)
    {
        return L"";
    }

    size_t size_s = strlen(src);
    size_t size_d = size_s + 10;

    wchar_t *des = new wchar_t[size_d];
    memset(des, 0, size_d * sizeof(wchar_t));

    size_t s = 0, d = 0;

    while (s  < size_s && d  < size_d)
    {
        unsigned char c = src[s];

        if ((c & 0x80) == 0)
        {
            des[d++] += src[s++];
        }
        else if((c & 0xE0) == 0xC0)
        {
            wchar_t& wideChar = des[d++];
            wideChar  = (src[s + 0] & 0x3F) << 6;
            wideChar |= (src[s + 1] & 0x3F);

            s += 2;
        }
        else if((c & 0xF0) == 0xE0)
        {
            wchar_t& wideChar = des[d++];

            wideChar  = (src[s + 0] & 0x1F) << 12;
            wideChar |= (src[s + 1] & 0x3F) << 6;
            wideChar |= (src[s + 2] & 0x3F);

            s += 3;
        }
        else if((c & 0xF8) == 0xF0)
        {
            wchar_t& wideChar = des[d++];

            wideChar  = (src[s + 0] & 0x0F) << 18;
            wideChar  = (src[s + 1] & 0x3F) << 12;
            wideChar |= (src[s + 2] & 0x3F) << 6;
            wideChar |= (src[s + 3] & 0x3F);

            s += 4;
        }
        else
        {
            wchar_t& wideChar = des[d++];

            wideChar  = (src[s + 0] & 0x07) << 24;
            wideChar  = (src[s + 1] & 0x3F) << 18;
            wideChar  = (src[s + 2] & 0x3F) << 12;
            wideChar |= (src[s + 3] & 0x3F) << 6;
            wideChar |= (src[s + 4] & 0x3F);

            s += 5;
        }
    }

    t = des;
    if (des != NULL)
    {
        delete[] des;
        des = NULL;
    }

    return t;
}

size_t Charset::uni2utf(const wstring& strRes, char* utf8, size_t nMaxSize)
{
    if (utf8 == NULL)
    {
        return -1;
    }

    size_t len = 0;
    size_t size_d = nMaxSize;


    for (wstring::const_iterator it = strRes.begin(); it != strRes.end(); ++it)
    {
        wchar_t wchar = *it;

        if (wchar < 0x80)
        {
            utf8[len++] = (char)wchar;
        }
        else if (wchar < 0x800)
        {
            if (len + 1 >= size_d)
            {
                return -1;
            }

            utf8[len++] = 0xc0 | (wchar >> 6);
            utf8[len++] = 0x80 | (wchar & 0x3f);
        }
        else if (wchar < 0x10000)
        {
            if (len + 2 >= size_d)
            {
                return -1;
            }

            utf8[len++] = 0xe0 | (wchar >> 12);
            utf8[len++] = 0x80 | ((wchar >> 6) & 0x3f);
            utf8[len++] = 0x80 | (wchar & 0x3f);
        }
        else if (wchar < 0x200000)
        {
            if (len + 3 >= size_d)
            {
                return -1;
            }

            utf8[len++] = 0xf0 | ((int)wchar >> 18);
            utf8[len++] = 0x80 | ((wchar >> 12) & 0x3f);
            utf8[len++] = 0x80 | ((wchar >> 6) & 0x3f);
            utf8[len++] = 0x80 | (wchar & 0x3f);
        }
    }

    return len;
}

string Charset::s2utfs(const string& strSrc)
{
    string strRes;
    wstring wstrUni = s2ws(strSrc);

    char* chUTF8 = new char[wstrUni.length() * 3];
    memset(chUTF8,0x00, wstrUni.length() * 3);
    uni2utf(wstrUni, chUTF8, wstrUni.length() * 3);
    strRes = chUTF8;

    if (chUTF8 != NULL)
    {
        delete[] chUTF8;
        chUTF8 = NULL;
    }

    return strRes;
}

string Charset::utfs2s(const string& strutf)
{
    wstring wStrTmp;
    utf2uni(strutf.c_str(), wStrTmp);

    return ws2s(wStrTmp);
}

string Charset::convert(const string& input, char* from_encoding, char* to_encoding)
{
#ifndef __ANDROID__
    size_t in_len  = input.length();
    size_t out_len = in_len * 4;
    iconv_t cd     = iconv_open(to_encoding, from_encoding);
    char* outbuf   = (char*)malloc(out_len);
    memset(outbuf, 0, out_len);

    char* in  = (char*)input.c_str();
    char* out = outbuf;

    iconv(cd, &in, (size_t*)&in_len, &out, &out_len);

    out_len = strlen(outbuf);
    string result(outbuf);
    free(outbuf);

    iconv_close(cd);

    return result;
#else
    return input;
#endif
}

}
}
