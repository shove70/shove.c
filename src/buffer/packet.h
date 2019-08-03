#pragma once

#include <type_traits>

#include "../utils/types.h"
#include "../utils/utils.h"
#include "../utils/bitmanip.h"
#include "../container/any.h"

#include "../hash/md5.h"
#include "../crypto/aes.h"
#include "../crypto/rsa.h"
#include "../crypto/tea.h"

using namespace std;
using namespace shove::utils;
using namespace shove::hash;
using namespace shove::crypto;
using namespace shove::container;

namespace shove
{
namespace buffer
{

enum CryptType
{
    NONE            = 0,
    XTEA            = 1,
    AES             = 2,
    RSA             = 3,
    RSA_XTEA_MIXIN  = 4
};

template <typename T> ubyte TypeID()
{
         if (is_same<typename std::decay<T>::type, char>   ::value)
        return 0x01;
    else if (is_same<typename std::decay<T>::type, ubyte>  ::value)
        return 0x02;
    else if (is_same<typename std::decay<T>::type, short>  ::value)
        return 0x03;
    else if (is_same<typename std::decay<T>::type, ushort> ::value)
        return 0x04;
    else if (is_same<typename std::decay<T>::type, int>    ::value)
        return 0x05;
    else if (is_same<typename std::decay<T>::type, uint>   ::value)
        return 0x06;
    else if (is_same<typename std::decay<T>::type, int64>  ::value)// || (is_same<typename std::decay<T>::type, long>::value))
        return 0x07;
    else if (is_same<typename std::decay<T>::type, uint64> ::value)// || (is_same<typename std::decay<T>::type, unsigned long>::value))
        return 0x08;
    else if (is_same<typename std::decay<T>::type, float>  ::value)
        return 0x20;
    else if (is_same<typename std::decay<T>::type, double> ::value)
        return 0x21;
    else if (is_same<typename std::decay<T>::type, float128> ::value)
        return 0x22;
    else if (is_same<typename std::decay<T>::type, bool>   ::value)
        return 0x30;
//    else if (is_same<typename std::decay<T>::type, char> ::value)
//        return 0x40;
    else if (is_same<typename std::decay<T>::type, string> ::value)// || is_same<typename std::decay<T>::type, char*> ::value || is_same<typename std::decay<T>::type, const char*>::value)
        return 0x41;
    else
    {
        assert(0);
    }
}

class BufferBuilder
{
private:
    vector<ubyte>* buffer;

public:
    BufferBuilder(vector<ubyte>*);
    void setBuffer(vector<ubyte>*);

    template <typename T>
    size_t put(const T& value, bool isWriteTypeInfo, bool isWriteLengthInfo, int lengthBytes)
    {
        assert(lengthBytes == 0 || lengthBytes == 2 || lengthBytes == 4);

        if (isWriteTypeInfo)
        {
            Bitmanip::write<ubyte>(TypeID<T>(), *buffer, buffer->size());
        }

        size_t len = sizeof(T);

        if (isWriteLengthInfo && lengthBytes > 0)
        {
            if (lengthBytes == 2)
                Bitmanip::write<ushort>((ushort)len, *buffer, buffer->size());
            else
                Bitmanip::write<uint>((uint)len, *buffer, buffer->size());
        }

        Bitmanip::write<T>(value, *buffer, buffer->size());

        return len;
    }
};

template <>
inline size_t BufferBuilder::put<string>(const string& value, bool isWriteTypeInfo, bool isWriteLengthInfo, int lengthBytes)
{
    assert(lengthBytes == 0 || lengthBytes == 2 || lengthBytes == 4);

    if (isWriteTypeInfo)
    {
        buffer->push_back(TypeID<string>());
    }

    size_t len = value.length();
    if (isWriteLengthInfo && lengthBytes > 0)
    {
        if (lengthBytes == 2)
            Bitmanip::write<ushort>((ushort)len, *buffer, buffer->size());
        else
            Bitmanip::write<uint>((uint)len, *buffer, buffer->size());
    }

    Bitmanip::write<string>(value, *buffer, buffer->size());

    return len;
}

template <>
inline size_t BufferBuilder::put<long>(const long& value, bool isWriteTypeInfo, bool isWriteLengthInfo, int lengthBytes)
{
    assert(lengthBytes == 0 || lengthBytes == 2 || lengthBytes == 4);

    if (isWriteTypeInfo)
    {
        Bitmanip::write<ubyte>(TypeID<int64>(), *buffer, buffer->size());
    }

    size_t len = sizeof(int64);

    if (isWriteLengthInfo && lengthBytes > 0)
    {
        if (lengthBytes == 2)
            Bitmanip::write<ushort>((ushort)len, *buffer, buffer->size());
        else
            Bitmanip::write<uint>((uint)len, *buffer, buffer->size());
    }

    Bitmanip::write<int64>((int64)value, *buffer, buffer->size());

    return len;
}

template <>
inline size_t BufferBuilder::put<unsigned long>(const unsigned long& value, bool isWriteTypeInfo, bool isWriteLengthInfo, int lengthBytes)
{
    assert(lengthBytes == 0 || lengthBytes == 2 || lengthBytes == 4);

    if (isWriteTypeInfo)
    {
        Bitmanip::write<ubyte>(TypeID<uint64>(), *buffer, buffer->size());
    }

    size_t len = sizeof(uint64);

    if (isWriteLengthInfo && lengthBytes > 0)
    {
        if (lengthBytes == 2)
            Bitmanip::write<ushort>((ushort)len, *buffer, buffer->size());
        else
            Bitmanip::write<uint>((uint)len, *buffer, buffer->size());
    }

    Bitmanip::write<uint64>((uint64)value, *buffer, buffer->size());

    return len;
}

class Packet
{
private:

    template <typename T>
    static void unpacker(BufferBuilder& bb, const T& t)
    {
        bb.put<T>(t, true, false, 0);
    }

    static void unpacker(BufferBuilder& bb)
    {
    }

    template <typename T>
    static void unpacker(BufferBuilder& bb, const T*& t)
    {
        assert(0);
    }

    template <typename T, typename... Params>
    static void unpacker(BufferBuilder& bb, const T& t, Params... params)
    {
        unpacker<T>(bb, t);
        unpacker(bb, params...);
    }

    template <typename T, typename... Params>
    static void unpacker(BufferBuilder& bb, const T*& t, Params... params)
    {
        unpacker<T>(bb, t);
        unpacker(bb, params...);
    }

public:
    template <typename... Params>
    static void build(vector<ubyte>& buffer, ushort magic, CryptType crypt, const string& key, const RSAKeyInfo& rsaKey, const string& name, const string& method, Params... params)
    {
        assert(name.length() <= 255);
        assert(method.length() <= 255);

        vector<ubyte> tlv;
        BufferBuilder bb(&tlv);
        unpacker(bb, params...);

        ubyte* tlv_p = tlv.data();
        size_t tlv_len = tlv.size();
        ubyte* en;
        size_t en_len;

        if (crypt == CryptType::NONE)
        {
            en = tlv_p;
            en_len = tlv_len;
        }
        else if (crypt == CryptType::XTEA)
        {
            int* xtea_key = (int*)key.c_str();
            en = new ubyte[tlv_len + 12];
            en_len = XTEAUtils::encrypt(tlv_p, tlv_len, xtea_key, en);
        }
        else if (crypt == CryptType::AES)
        {
            en = new ubyte[tlv_len + 20];
            en_len = AESUtils::encrypt<AES128>(tlv_p, tlv_len, key, en);
        }
        else if (crypt == CryptType::RSA)
        {
            en = new ubyte[tlv_len * 2 + (rsaKey.modulus.dataLength << 2)];
            en_len = RSA::encrypt(rsaKey, tlv_p, tlv_len, en);
        }
        else
        {   //  CryptType::RSA_XTEA_MIXIN
            en = new ubyte[tlv_len * 2 + 12 + (rsaKey.modulus.dataLength << 2)];
            en_len = RSA::encrypt(rsaKey, tlv_p, tlv_len, en, true);
        }

        buffer.clear();
        bb.setBuffer(&buffer);

        bb.put<ushort>(magic, false, false, 0);
        bb.put<int>(0, false, false, 0);     // length, seize a seat.
        bb.put<string>(name, false, true, 2);
        bb.put<string>(method, false, true, 2);
        Bitmanip::write<int>((int)(buffer.size() - 2 - 4 + en_len + 2), buffer, 2);
        buffer.insert(buffer.end(), en, en + en_len);

        if (crypt != CryptType::NONE) delete[] en;

        string md5 = MD5Utils.GenerateMD5(buffer.data(), buffer.size());
        ubyte md5_buf[2];
        strToByte_hex(md5, md5_buf, 4);
        buffer.insert(buffer.end(), md5_buf, md5_buf + 2);
    }

    static size_t parseInfo(ubyte* buffer, size_t len, string& name, string& method);
    static void parse(vector<Any>& result, ubyte* buffer, size_t len, ushort magic, CryptType crypt, const string& key, const RSAKeyInfo& rsaKey, string& name, string& method);
};

template <>
inline void Packet::unpacker<string>(BufferBuilder& bb, const string& t)
{
    bb.put<string>(t, true, true, 4);
}

template <>
inline void Packet::unpacker<char>(BufferBuilder& bb, const char*& t)
{
    string s(t);
    bb.put<string>(s, true, true, 4);
}

template <>
inline void Packet::unpacker<const char>(BufferBuilder& bb, const char*& t)
{
    string s(t);
    bb.put<string>(s, true, true, 4);
}

}
}
