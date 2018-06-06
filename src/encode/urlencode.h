#pragma once

#include <iostream>
#include <string>

#include "../utils/types.h"

using namespace std;

namespace shove
{
namespace encode
{

class UrlEncode
{
public:

    static string encode(const string& str);
    static string decode(const string& str);

private:

    static ubyte ToHex  (ubyte x);
    static ubyte FromHex(ubyte x);
};

}
}
