#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <sstream>
#include <cctype>
#include <cstring>

using namespace std;

namespace shove
{

enum JsonType
{
    JSON_EMPTY, JSON_INTEGER, JSON_DOUBLE, JSON_BOOLEAN, JSON_STRING, JSON_ARRAY, JSON_OBJECT
};

class Json
{
public:
    Json() : _type(JSON_EMPTY)
    {
    };

    Json(short v) : _type(JSON_INTEGER), v_int64(v)
    {
    };

    Json(unsigned short v) : _type(JSON_INTEGER), v_int64(v)
    {
    };

    Json(int v) : _type(JSON_INTEGER), v_int64(v)
    {
    };

    Json(unsigned int v) : _type(JSON_INTEGER), v_int64(v)
    {
    };

    Json(long long v) : _type(JSON_INTEGER), v_int64(v)
    {
    };

    Json(unsigned long long v) : _type(JSON_INTEGER), v_int64(v)
    {
    };

    Json(float v) : _type(JSON_DOUBLE), v_double(v)
    {
    };

    Json(double v) : _type(JSON_DOUBLE), v_double(v)
    {
    };

    Json(bool v) : _type(JSON_BOOLEAN), v_bool(v)
    {
    };

    Json(const string& v) : _type(JSON_STRING), v_string(v)
    {
    };

    Json(string&& v) : _type(JSON_STRING), v_string(move(v))
    {
    };

    Json(const char* v) : _type(JSON_STRING), v_string(v)
    {
    };

    Json(const vector<Json>& v);
    Json(vector<Json>&& v);
    Json(const map<string, Json>& v);
    Json(map<string, Json>&& v);

    short asInt16() const;
    unsigned short asUint16() const;
    int asInt32() const;
    unsigned int asUint32() const;
    long long asInt64() const;
    unsigned long long asUint64() const;
    float asFloat() const;
    double asDouble() const;
    bool asBool() const;
    const string& asString() const;
    vector<Json>& asVector();
    const vector<Json>& asVector() const;
    map<string, Json>& asMap();
    const map<string, Json>& asMap() const;
    JsonType type() const;

    Json& operator[](size_t i);
    const Json& operator[](size_t i) const;
    Json& operator[](const string& key);
    const Json& operator[](const string& key) const;

    Json& parse(const string& in);
    string stringify() const;

    friend ostream& operator<< (ostream& os, const Json& v)
    {
        os << v.stringify();
        return os;
    }

private:

    JsonType _type;

    union
    {
        long long v_int64;
        double v_double;
        bool v_bool;
    };

    string v_string;
    shared_ptr<vector<Json>> v_arr;
    shared_ptr<map<string, Json>> v_obj;

    void check_type(const JsonType& t) const;
    void rec_write(ostringstream& O) const;
};

typedef map<string, Json> JObject;
typedef vector<Json> JArray;

}
