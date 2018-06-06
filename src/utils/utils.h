#pragma once

#include <iostream>
#include <vector>
#include <sstream>

#include "../text/regex.h"

#include "types.h"

using namespace std;
using namespace shove::regex;

namespace shove
{
namespace utils
{

void splitString(const string& str, vector<string>& result, const string& delimiter);

struct SplitResult
{
    string::size_type pos;
    string::size_type count;

    inline SplitResult(string::size_type pos, string::size_type count)
    {
        this->pos   = pos;
        this->count = count;
    }
};
void splitString(const string& str, vector<SplitResult>& result, const CRegexpT<char>& re);

void replaceString(string& str, const string& src, const string& dst, int count);
string trim(string str);
bool startsWith(const string& str, const string& start);
bool endsWith(const string& str, const string& end);
string stringCut_utf8(const string& str, size_t len);

bool downloadFile(const string& url, const string& saveFilename);
string readFile(const string& filename);
void writeFile(const string& filename, const string& data);
void writeFile(const string& filename, char* data, size_t len);

string timeToString(time_t t);
time_t timeFromString(const string& str);

template<typename T>
string vectorToString(const vector<typename enable_if<is_same<char,  T>::value || is_same<ubyte, T>::value, T>::type>& buf)
{
    char* data = new char[buf.size()];
    for (size_t i = 0; i < buf.size(); i++)
    {
        data[i] = (char)buf[i];
    }

    string ret(data, buf.size());
    delete[] data;

    return ret;
}

template<typename T>
void stringToVector(const string& input, vector<typename enable_if<is_same<char,  T>::value || is_same<ubyte, T>::value, T>::type>& result)
{
    result.clear();

    for (auto x: input)
    {
        result.push_back((T)x);
    }
}

template<typename T>
string toString(T value)
{
    stringstream ss;
    ss << value;

    return trim(ss.str());
}
template<>
inline string toString<char>(char value)
{
    stringstream ss;
    ss << (int)value;

    return trim(ss.str());
}
template<>
inline string toString<ubyte>(ubyte value)
{
    stringstream ss;
    ss << (unsigned int)value;

    return trim(ss.str());
}

void _sleep(int seconds);

template<typename T>
bool inArray(vector<T> array, T value)
{
    return (count(array.begin(), array.end(), value) > 0);
}

size_t strToByte_hex(const string&, ubyte*, size_t = 0);
string byteToStr_hex(ubyte*, size_t);

string rsaEncrypt(const string& key, const string& data);
string rsaDecrypt(const string& key, const string& data);

string fileMD5(const string& filename);

}
}
