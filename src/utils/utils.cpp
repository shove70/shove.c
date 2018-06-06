#ifdef _MSC_VER
    #include <urlmon.h>
#else
    #include <curl/curl.h>
#endif
#include <iostream>
#include <fstream>
#include <time.h>
#include <cassert>
#ifdef _MSC_VER
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#include <string>
    #pragma comment(lib, "urlmon.lib")
#else
    #include <unistd.h>
#endif

#include "../crypto/rsa.h"
#include "../encode/base64.h"
#include "../hash/md5.h"

#include "../io/filesystem.h"
#include "utils.h"

using namespace shove::io;
using namespace shove::crypto;
using namespace shove::encode;

namespace shove
{
namespace utils
{

void splitString(const string& str, vector<string>& result, const string& delimiter)
{
    result.clear();
    string::size_type pos1, pos2;
    pos2 = str.find(delimiter);
    pos1 = 0;

    while (string::npos != pos2)
    {
        result.push_back(str.substr(pos1, pos2 - pos1));

        pos1 = pos2 + delimiter.size();
        pos2 = str.find(delimiter, pos1);
    }

    if (pos1 != str.length())
        result.push_back(str.substr(pos1));
}

void splitString(const string& str, vector<SplitResult>& result, const CRegexpT<char>& re)
{
    result.clear();
    string::size_type pos1, pos2;
    pos1 = 0;

    MatchResult match = re.Match(str.c_str());

    while (match.IsMatched())
    {
        pos2 = match.GetStart();
        result.push_back(SplitResult(pos1, pos2 - pos1));

        pos1 = pos2 + (match.GetEnd() - match.GetStart());
        match = re.Match(str.c_str(), match.GetEnd());
    }

    if (pos1 != str.length())
        result.push_back(SplitResult(pos1, str.length() - pos1));
}

void replaceString(string& str, const string& src, const string& dst, int count)
{
    string::size_type pos = 0;
    string::size_type src_len = src.size();
    string::size_type dst_len = dst.size();

    int times = 0;
    while ((pos = str.find(src, pos)) != string::npos)
    {
        str.replace(pos, src_len, dst);
        pos += dst_len;
        times++;
        if ((count > 0) && (times == count))
        {
            return;
        }
    }
}

string trim(string str)
{
    if (str.empty())
    {
        return str;
    }

    str.erase(0,str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);

    return str;
}

bool startsWith(const string& str, const string& start)
{
    size_t srclen   = str.size();
    size_t startlen = start.size();

    if(srclen < startlen)
    {
        return false;
    }

    return (str.substr(0, startlen) == start);
}

bool endsWith(const string& str, const string& end)
{
    size_t srclen = str.size();
    size_t endlen = end.size();

    if(srclen < endlen)
    {
        return false;
    }

    return (str.substr(srclen - endlen, endlen) == end);
}

string stringCut_utf8(const string& str, size_t len)
{
    if ((len == 0) || (str.size() == 0))
    {
        return "";
    }

    size_t alreadyLen = 0;

    size_t i = 0;
    while (alreadyLen < len && i < str.size())
    {
        unsigned int c = (ubyte)str[i];

        if (c <= 127)
        {
            alreadyLen++;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            alreadyLen += 2;
            i += 1;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            alreadyLen += 2;
            i += 2;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            alreadyLen += 2;
            i += 3;
        }
        i++;
    }

    return str.substr(0, i);
}

bool downloadFile(const string& url, const string& saveFilename)
{
    string path = dirName(saveFilename);

    if (!fileExists(path))
    {
        createDirectories(path);
    }

#ifdef _MSC_VER
#ifdef UNICODE
    int len = MultiByteToWideChar(CP_ACP, 0, url.c_str(), -1, NULL, 0);
    TCHAR* ansi_url = new TCHAR[len + 1];
    memset(ansi_url, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, url.c_str(), -1, ansi_url, len);

    len = MultiByteToWideChar(CP_ACP, 0, saveFilename.c_str(), -1, NULL, 0);
    TCHAR* ansi_saveFilename = new TCHAR[len + 1 + 20];
    memset(ansi_saveFilename, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, saveFilename.c_str(), -1, ansi_saveFilename, len);

    char* temp = ".tmp";
    TCHAR wideTemp[20];
    MultiByteToWideChar(CP_ACP, 0, temp, -1, wideTemp, 20);
    memcpy(ansi_saveFilename + len, wideTemp, 20);
    
    int iLen = WideCharToMultiByte(CP_ACP, 0, ansi_saveFilename, -1, NULL, 0, NULL, NULL);
    char* chRtn = new char[iLen + 1];
    WideCharToMultiByte(CP_ACP, 0, ansi_saveFilename, -1, chRtn, iLen, NULL, NULL);
    string tempfile(chRtn);

    bool result = (URLDownloadToFile(0, ansi_url, ansi_saveFilename, 0, NULL) == S_OK);

    if (result && fileExists(tempfile) && (fileLength(tempfile) > 0))
    {
        copyFile(tempfile, std::string(chRtn));
        remove(tempfile.c_str());
    }

    delete[] ansi_url;
    delete[] ansi_saveFilename;
    delete[] chRtn;

    return result;
#else
    string tempfile = saveFilename + ".tmp";
    bool result = (URLDownloadToFile(0, url.c_str(), tempfile.c_str(), 0, NULL) == S_OK);

    if (result && fileExists(tempfile) && (fileLength(tempfile) > 0))
    {
        copyFile(tempfile, saveFilename);
        remove(tempfile.c_str());
    }

    return result;
#endif
#else
    string tempfile = saveFilename + ".tmp";
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    FILE* fp = fopen(tempfile.c_str(), "wb");
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    res = curl_easy_perform(curl);
    fclose(fp);

    bool result = (res == CURLE_OK); // CURLE_WRITE_ERROR
    curl_easy_cleanup(curl);

    if (result && fileExists(tempfile) && (fileLength(tempfile) > 0))
    {
        copyFile(tempfile, saveFilename);
        remove(tempfile.c_str());
    }

    return result;
#endif
}

string readFile(const string& filename)
{
    ifstream file;
    file.open(filename, ios::binary);
    long len = 0;
    string data;
    if (file.is_open())
    {
        file.seekg(0, ios::end);
        len = (long)file.tellg();
        file.seekg(0, ios::beg);
        char* buf = new char[len + 1];
        file.read(buf, len);
        file.close();

        data = string(buf, len);
        delete[] buf;
    }

    return data;
}

void writeFile(const string& filename, const string& data)
{
    writeFile(filename, (char*)data.c_str(), data.size());
}

void writeFile(const string& filename, char* data, size_t len)
{
    ofstream file;
    file.open(filename, ios::binary);
    file.write(data, len);
    file.close();
}

//string timeToString(time_t t)   // 2016-04-07 11:58:00
//{
//    struct tm t2;
//
//#ifdef WIN32
//    tm1 = *localtime(&t);
//#else
//    localtime_r(&t, &t2);
//#endif
//
//    char* szTime = new char[20];
//    sprintf(szTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", t2.tm_year + 1900, t2.tm_mon + 1, t2.tm_mday, t2.tm_hour, t2.tm_min, t2.tm_sec);
//    szTime[19] = '\0';
//    string ret(szTime);
//    delete[] szTime;
//
//    return ret;
//}

string timeToString(time_t t)   // 2016-04-07 11:58:00
{
    char s[20];
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", localtime(&t));
    string result(s);

    return result;
}

time_t timeFromString(const string& str)    // 2008-12-29 23:54:00
{
    time_t t1;
    struct tm t2;

    sscanf(str.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", &t2.tm_year, &t2.tm_mon, &t2.tm_mday, &t2.tm_hour, &t2.tm_min, &t2.tm_sec);

    t2.tm_year -= 1900;
    t2.tm_mon--;
    t2.tm_isdst = -1;

    t1 = mktime(&t2);
    return t1;
}

// not strptime, timelocal on windows.
//time_t timeFromString(const string& str)    // 2008-12-29 23:54:00
//{
//    struct tm t;
//    strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &t);
//    return timelocal(&t);
//}

void _sleep(int seconds)
{
#ifdef _MSC_VER
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif
}

size_t strToByte_hex(const string& input, ubyte* result, size_t max_len)
{
    size_t len = ((max_len == 0) ? input.length() : max_len) / 2;
    char* p = (char*)input.c_str();

    for (size_t i = 0 ; i < len; i++)
    {
        char t[3];
        t[0] = p[i * 2];
        t[1] = p[i * 2 + 1];
        t[2] = 0;
        uint b;
        sscanf(t, "%X", &b);
        result[i] = (ubyte)b;
    }

    return len;
}

string byteToStr_hex(ubyte* input, size_t len)
{
    char* t = new char[len * 2];

    for (size_t i = 0; i < len; i++)
    {
        sprintf(t + i * 2, "%02X", input[i]);
    }

    string ret(t, len * 2);
    delete[] t;

    return ret;
}

string rsaEncrypt(const string& key, const string& data)
{
    unsigned char* buf = new unsigned char[data.size() * 2 + key.size()];
    unsigned char* p = (unsigned char*)data.c_str();
    size_t len = RSA::encrypt(key, p, data.size(), buf, true);

    string ret = Base64::encode(buf, len);
    delete[] buf;

    return ret;
}

string rsaDecrypt(const string& key, const string& data)
{
    unsigned char* buf = new unsigned char[data.size() * 2];
    size_t len = Base64::decode(data, buf);

    unsigned char* buf2 = new unsigned char[len * 2 + key.size()];
    len = RSA::decrypt(key, buf, len, buf2, true);

    string ret((char*)buf2, len);
    delete[] buf;
    delete[] buf2;

    return ret;
}


}
}
