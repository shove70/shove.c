#pragma once

#include <vector>
#include <cassert>
#include <istream>

#include "types.h"

using namespace std;

namespace shove
{
namespace utils
{

class Bitmanip
{
public:
    template <typename T>
    static size_t write(const T& value, vector<ubyte>& buffer, size_t offset)
    {
        while (buffer.size() < (offset + sizeof(T)))
        {
            buffer.push_back((ubyte)0);
        }

        ubyte* p = (ubyte*)&value;

        size_t i = 0;
        while (i < sizeof(T))
        {
            buffer[offset + sizeof(T) - i - 1] = p[i];
            i++;
        }

        return sizeof(T);
    }

    template <typename T>
    static size_t write(const T& value, ubyte* buffer, size_t offset)
    {
        ubyte* p = (ubyte*)&value;

        size_t i = 0;
        while (i < sizeof(T))
        {
            buffer[offset + sizeof(T) - i - 1] = p[i];
            i++;
        }

        return sizeof(T);
    }

    template <typename T>
    static T peek(ubyte* buffer, size_t offset, size_t count = 0)
    {
        T t;
        ubyte* p = (ubyte*)&t;

        size_t i = 0;
        while (i < sizeof(T))
        {
            p[i] = buffer[offset + sizeof(T) - i - 1];
            i++;
        }

        return t;
    }
};

template <>
inline size_t Bitmanip::write<string>(const string& value, vector<ubyte>& buffer, size_t offset)
{
    while (buffer.size() < offset + value.length())
    {
        buffer.push_back((ubyte)0);
    }

    ubyte* p = (ubyte*)value.c_str();

    size_t i = 0;
    while (i < value.length())
    {
        buffer[offset + i] = p[i];
        i++;
    }

    return (uint)value.length();
}

template <>
inline string Bitmanip::peek<string>(ubyte* buffer, size_t offset, size_t count)
{
    string ret(count, 0);

    for (size_t i = 0; i < count; i++)
    {
        ((char*)ret.c_str())[i] = buffer[offset + i];
    }

    return ret;
}

template <>
inline size_t Bitmanip::write<long>(const long& value, vector<ubyte>& buffer, size_t offset)
{
    int64 v = value;

    while (buffer.size() < offset + sizeof(int64))
    {
        buffer.push_back((ubyte)0);
    }

    ubyte* p = (ubyte*)&v;

    size_t i = 0;
    while (i < sizeof(int64))
    {
        buffer[offset + sizeof(int64) - i - 1] = p[i];
        i++;
    }

    return sizeof(int64);
}

template <>
inline long Bitmanip::peek<long>(ubyte* buffer, size_t offset, size_t count)
{
    int64 t;
    ubyte* p = (ubyte*)&t;

    size_t i = 0;
    while (i < sizeof(int64))
    {
        p[i] = buffer[offset + sizeof(int64) - i - 1];
        i++;
    }

    return (long)t;
}

template <>
inline size_t Bitmanip::write<unsigned long>(const unsigned long& value, vector<ubyte>& buffer, size_t offset)
{
    uint64 v = value;

    while (buffer.size() < offset + sizeof(uint64))
    {
        buffer.push_back((ubyte)0);
    }

    ubyte* p = (ubyte*)&v;

    size_t i = 0;
    while (i < sizeof(uint64))
    {
        buffer[offset + sizeof(uint64) - i - 1] = p[i];
        i++;
    }

    return sizeof(uint64);
}

template <>
inline unsigned long Bitmanip::peek<unsigned long>(ubyte* buffer, size_t offset, size_t count)
{
    uint64 t;
    ubyte* p = (ubyte*)&t;

    size_t i = 0;
    while (i < sizeof(uint64))
    {
        p[i] = buffer[offset + sizeof(uint64) - i - 1];
        i++;
    }

    return (unsigned long)t;
}

}
}
