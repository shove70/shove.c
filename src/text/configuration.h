#pragma once

#include <string>
#include <stdio.h>
#include <map>
#include <cstdlib>
#include <sstream>

using namespace std;

namespace shove
{

class Configuration
{

public:

    Configuration(const string& filename);

    template<typename T>
    typename enable_if<is_same<short,                T>::value ||
                       is_same<unsigned short,       T>::value ||
                       is_same<int,                  T>::value ||
                       is_same<unsigned int,         T>::value ||
                       is_same<long,                 T>::value ||
                       is_same<unsigned long,        T>::value ||
                       is_same<long long,            T>::value ||
                       is_same<unsigned long long,   T>::value ||
                       is_same<float,                T>::value ||
                       is_same<double,               T>::value ||
                       is_same<long double,          T>::value ||
                       is_same<bool,                 T>::value ||
                       is_same<string,               T>::value, T>::type get(const string& key)
    {
        //assert(data.find(key) != data.end());
        string value = data.at(key);
        if (value.empty()) value = "0";

        T number = 0;
        std::stringstream ss;
        ss << value;
        ss >> number;

        return number;
    }

private:

    map<string, string> data;
    string trim_all(const string& input);
};

template<>
inline string Configuration::get<string>(const string& key)
{
    //assert(data.find(key) != data.end());
    return data.at(key);
}

template<>
inline bool Configuration::get<bool>(const string& key)
{
    //assert(data.find(key) != data.end());
    string value = data.at(key);
    return (value == "1" || value == "true" || value == "True" || value == "TRUE");
}

}
