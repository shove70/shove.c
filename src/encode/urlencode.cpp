#include <cassert>

#include "urlencode.h"

namespace shove
{
namespace encode
{

ubyte UrlEncode::ToHex(ubyte x)
{
    return  x > 9 ? x + 55 : x + 48;
}

ubyte UrlEncode::FromHex(ubyte x)
{
    ubyte y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

string UrlEncode::encode(const string& str)
{
    string result = "";
    size_t length = str.length();

    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            result += str[i];
        else if (str[i] == ' ')
        {
            result += "+";
        }
        else
        {
            result += '%';
            result += ToHex((unsigned char)str[i] >> 4);
            result += ToHex((unsigned char)str[i] % 16);
        }
    }

    return result;
}

string UrlEncode::decode(const string& str)
{
    string result = "";
    size_t length = str.length();

    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+')
        {
            result += ' ';
        }
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            ubyte high = FromHex((ubyte)str[++i]);
            ubyte low  = FromHex((ubyte)str[++i]);
            result += high * 16 + low;
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}

}
}
