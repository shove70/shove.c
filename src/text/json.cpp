#include <stdexcept>

#include "json.h"

namespace shove
{

Json::Json(const vector<Json>& v)
{
    this->_type = JSON_ARRAY;
    this->v_arr = shared_ptr<vector<Json>>(new vector<Json>());
    *(this->v_arr) = v;
}

Json::Json(vector<Json>&& v)
{
    this->_type = JSON_ARRAY;
    this->v_arr = shared_ptr<vector<Json>>(new vector<Json>());
    *(this->v_arr) = move(v);
}

Json::Json(const map<string, Json>& v)
{
    this->_type = JSON_OBJECT;
    this->v_obj = shared_ptr<map<string, Json>>(new map<string, Json>());
    *(this->v_obj) = v;
}

Json::Json(map<string, Json>&& v)
{
    this->_type = JSON_OBJECT;
    this->v_obj = shared_ptr<map<string, Json>>(new map<string, Json>());
    *(this->v_obj) = move(v);
}

void Json::check_type(const JsonType& t) const
{
    if (t != this->_type)
    {
        throw runtime_error("type error");
    }
}

short Json::asInt16() const
{
    check_type(JSON_INTEGER);

    if ((short)this->v_int64 != this->v_int64)
    {
        throw runtime_error("Integer is too big than int16.");
    }

    return (short)this->v_int64;
}

unsigned short Json::asUint16() const
{
    check_type(JSON_INTEGER);

    if ((unsigned short)this->v_int64 != this->v_int64)
    {
        throw runtime_error("Integer is too big than uint16.");
    }

    return (unsigned short)this->v_int64;
}

int Json::asInt32() const
{
    check_type(JSON_INTEGER);

    if ((int)this->v_int64 != this->v_int64)
    {
        throw runtime_error("Integer is too big than int32.");
    }

    return (int)this->v_int64;
}

unsigned int Json::asUint32() const
{
    check_type(JSON_INTEGER);

    if ((unsigned int)this->v_int64 != this->v_int64)
    {
        throw runtime_error("Integer is too big than uint32.");
    }

    return (unsigned int)this->v_int64;
}

long long Json::asInt64() const
{
    check_type(JSON_INTEGER);
    return this->v_int64;
}

unsigned long long Json::asUint64() const
{
    check_type(JSON_INTEGER);
    return (unsigned long long)this->v_int64;
}

float Json::asFloat() const
{
    if (_type == JSON_INTEGER)
    {
        if ((float)this->v_int64 != this->v_int64)
        {
            throw runtime_error("Integer is too big than float.");
        }
        return (float)this->v_int64;
    }

    check_type(JSON_DOUBLE);
    if ((float)this->v_double != this->v_double)
    {
        throw runtime_error("Double is too big than float.");
    }
    return (float)this->v_double;
}

double Json::asDouble() const
{
    if (_type == JSON_INTEGER)
    {
        return (double)this->v_int64;
    }

    check_type(JSON_DOUBLE);
    return this->v_double;
}

bool Json::asBool() const
{
    check_type(JSON_BOOLEAN);
    return this->v_bool;
}

const string& Json::asString() const
{
    check_type(JSON_STRING);
    return this->v_string;
}

vector<Json>& Json::asVector()
{
    check_type(JSON_ARRAY);
    return *(this->v_arr);
}

const vector<Json>& Json::asVector() const
{
    check_type(JSON_ARRAY);
    return *(this->v_arr);
}

map<string, Json>& Json::asMap()
{
    check_type(JSON_OBJECT);
    return *(this->v_obj);
}

const map<string, Json>& Json::asMap() const
{
    check_type(JSON_OBJECT);
    return *(this->v_obj);
}

JsonType Json::type() const
{
    return this->_type;
}

Json& Json::operator[](size_t i)
{
    check_type(JSON_ARRAY);
    return (*(this->v_arr))[i];
}

const Json& Json::operator[](size_t i) const
{
    check_type(JSON_ARRAY);
    return (*(this->v_arr))[i];
}

Json& Json::operator[](const string& key)
{
    check_type(JSON_OBJECT);
    return (*(this->v_obj))[key];
}

const Json& Json::operator[](const string& key) const
{
    check_type(JSON_OBJECT);
    auto v = this->v_obj->find(key);

    if (v == this->v_obj->end())
    {
        throw runtime_error("key: " + key + " missed.");
    }

    return v->second;
}

Json& Json::parse(const string& in)
{
    vector<pair<char, Json>> sta;
    sta.reserve(10000);
    const char* p = in.c_str();

    for (size_t i = 0; i < in.size(); ++i)
    {
        if (isspace(p[i]) || p[i] == '-' || p[i] == ':' || p[i] == ',')
        {
            continue;
        }
        if (p[i] == '[')
        {
            sta.emplace_back('[', Json());
        }
        else if (p[i] == '{')
        {
            sta.emplace_back('{', Json());
        }
        else if (p[i] == ']')
        {
            Json now { vector<Json>() };
            auto& vec = now.asVector();

            while (!sta.empty() && sta.back().first != '[')
            {
                vec.emplace_back(move(sta.back().second));
                sta.pop_back();
            }

            reverse(vec.begin(), vec.end());

            if (sta.empty() || sta.back().first != '[')
            {
                throw runtime_error("parse error, [] not match.");
            }

            sta.pop_back();
            sta.emplace_back('a', move(now));
        }
        else if (p[i] == '}')
        {
            Json now { map<string, Json>() };

            while (sta.size() >= 2 && sta.back().first != '{')
            {
                pair<string, Json> a;
                a.second = move(sta.back().second);
                sta.pop_back();

                if (sta.back().second.type() != JSON_STRING)
                {
                    throw runtime_error("parse error: some of the keys are not string.");
                }

                a.first = move(sta.back().second.v_string);
                sta.pop_back();
                now.asMap().insert(a);
            }

            if (sta.empty() || sta.back().first != '{')
            {
                throw runtime_error("parse error, {} not match.");
            }

            sta.pop_back();
            sta.emplace_back('o', move(now));
        }
        else if (p[i] == '"')
        {
            size_t j = i + 1;
            string S;
            S.reserve(100);

            while (j < in.size() && p[j] != '"')
            {
                if (p[j] == '\\')
                {
                    if (p[j + 1] == 'b')
                    {
                        S.push_back('\b');
                    }
                    else if (p[j + 1] == 'f')
                    {
                        S.push_back('\f');
                    }
                    else if (p[j + 1] == 'n')
                    {
                        S.push_back('\n');
                    }
                    else if (p[j + 1] == 'r')
                    {
                        S.push_back('\r');
                    }
                    else if (p[j + 1] == 't')
                    {
                        S.push_back('\t');
                    }
                    else
                    {
                        S.push_back(p[j + 1]);
                    }
                    j += 2;
                }
                else
                {
                    S.push_back(p[j]);
                    j += 1;
                }
            }
            sta.emplace_back('v', Json(move(S)));
            i = j;
        }
        else if (isdigit(p[i]))
        {
            bool isdouble = false;

            for (auto j = i; j < in.size() && isdigit(p[j]); j += 1)
            {
                if (p[j + 1] == '.' || p[j + 1] == 'e' || p[j + 1] == 'E')
                {
                    isdouble = true;
                    break;
                }
            }

            int minus_flag = 1;

            if (i > 0 && p[i - 1] == '-')
            {
                minus_flag = -1;
            }

            if (isdouble)
            {
                double v = 0;
                sscanf(p + i, "%lf", &v);
                v *= minus_flag;
                sta.emplace_back('d', Json(v));
            }
            else
            {
                long long v = 0;
#ifdef __linux
                sscanf(p + i, "%ld", (long*)&v);
#else
                sscanf(p + i, "%lld", &v);
#endif
                v *= minus_flag;
                sta.emplace_back('i', Json(v));
            }

            while (i < in.size() && p[i] != ',' && p[i] != ']' && p[i] != '}')
            {
                i += 1;
                continue;
            }
            i -= 1;
        }
        else if (strncmp("null", p + i, 4) == 0)
        {
            sta.emplace_back('n', Json());
            i += 3;
        }
        else if (strncmp("true", p + i, 4) == 0)
        {
            sta.emplace_back('b', Json(true));
            i += 3;
        }
        else if (strncmp("false", p + i, 4) == 0)
        {
            sta.emplace_back('b', Json(false));
            i += 4;
        }
        else
        {
            throw runtime_error(string("parse error:unrecognized character.") + (p + i));
        }
    }

    if (sta.size() == 1)
    {
        *this = move(sta.back().second);
    }
    else
    {
        throw runtime_error("parse error, type = final.");
    }

    return *this;
}

void Json::rec_write(ostringstream& O) const
{
    if (this->_type == JSON_EMPTY)
    {
        O << "null";
    }
    else if (this->_type == JSON_INTEGER)
    {
        O << this->v_int64;
    }
    else if (this->_type == JSON_DOUBLE)
    {
        O << this->v_double;
    }
    else if (this->_type == JSON_BOOLEAN)
    {
        if (this->v_bool == true)
        {
            O << "true";
        }
        else
        {
            O << "false";
        }
    }
    else if (this->_type == JSON_STRING)
    {
        O << '"';

        for (const auto& each : this->v_string)
        {
            if (each == '\\' || each == '"')
            {
                O << '\\' << each;
            }
            else if (each == '\b')
            {
                O << '\\' << 'b';
            }
            else if (each == '\f')
            {
                O << '\\' << 'f';
            }
            else if (each == '\n')
            {
                O << '\\' << 'n';
            }
            else if (each == '\r')
            {
                O << '\\' << 'r';
            }
            else if (each == '\t')
            {
                O << '\\' << 't';
            }
            else
            {
                O << each;
            }
        }

        O << '"';
    }
    else if (_type == JSON_ARRAY)
    {
        O << '[';

        for (auto it = this->v_arr->begin(); it != this->v_arr->end(); ++it)
        {
            if (it != this->v_arr->begin())
            {
                O << ',';
            }

            it->rec_write(O);
        }

        O << ']';
    }
    else if (_type == JSON_OBJECT)
    {
        O << '{';

        for (auto it = this->v_obj->begin(); it != this->v_obj->end(); ++it)
        {
            if (it != this->v_obj->begin())
            {
                O << ',';
            }

            O << '"' << it->first << '"' << ':';
            it->second.rec_write(O);
        }

        O << '}';
    }
}

string Json::stringify() const
{
    ostringstream O;
    rec_write(O);

    return O.str();
}

}

/*
    string str = "{\"hello\":[\"json\", \"momo\", 123]}";
    auto json = Json().parse(str);
    cout << json << endl;
    cout << json["hello"][2].asInt64() << endl;
    cout << json["hello"][2].asFloat() << endl;

    json = Json(JObject{
            { "hello", "json" },
            { "arr", JArray{1, 2, 3} }
            });
    cout << json["hello"] << endl;
    cout << json << endl;

    string s = json.stringify();
    cout << s;
*/
