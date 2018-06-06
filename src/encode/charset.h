#include <iostream>

using namespace std;

namespace shove
{
namespace encode
{

class Charset
{
public:

    static string  ws2s   (const wstring& ws);
    static wstring s2ws   (const string& s);
    static wstring utf2uni(const char* src, wstring& t);
    static int     uni2utf(const wstring& strRes, char* utf8, int nMaxSize);
    static string  s2utfs (const string& strSrc);
    static string  utfs2s (const string& strutf);

    //static string  convert(const string& input, char* from_encoding, char* to_encoding);
};

}
}
