#pragma once

#include "packet.h"

namespace shove
{
namespace buffer
{

extern ushort       _magic;
extern CryptType    _crypt;
extern string       _key;
extern RSAKeyInfo   _rsaKey;

class Message
{

protected:

    typedef char    int8;
    typedef ubyte   uint8;
    typedef short   int16;
    typedef ushort  uint16;
    typedef int     int32;
    typedef uint    uint32;
    //int64
    //uint64
    typedef float   float32;
    typedef double  float64;
    //float128(real)
    //bool
    //char(repeated)
    //string

public:

    static void settings(ushort magic, CryptType crypt = CryptType::NONE, string key = "");
    static void settings(ushort magic, RSAKeyInfo rsaKey, bool mixinXteaMode = false);

    template <typename... Params>
    static void serialize_without_msginfo(vector<ubyte>& buffer, string method, Params... params)
    {
        Packet::build(buffer, _magic, _crypt, _key, _rsaKey, "", method, params...);
    }

    static void getMessageInfo(ubyte* buffer, size_t len, string& name, string& method);
    static void deserialize(vector<Any>& result, ubyte* buffer, size_t len, string& name, string& method);

    template <class T>
    static typename enable_if<is_base_of<Message, T>::value, T>::type deserialize(ubyte* buffer, size_t len)
    {
        string method;

        return deserialize<T>(buffer, len, method);
    }

    template <class T>
    static typename enable_if<is_base_of<Message, T>::value, T>::type deserialize(ubyte* buffer, size_t len, string& method)
    {
        string name;
        vector<Any> params;
        deserialize(params, buffer, len, name, method);

        if (name.empty() || params.size() == 0)
        {
            cout << "Invalid message buffer." << endl;
            throw;
        }

        T message;
        if (message._className() != name)
        {
            cout << "The type " << name << " of the incoming template is incorrect." << endl;
            throw;
        }

        message.setValue(params);

        return message;
    }

protected:

    template <typename... Params>
    void serialize(vector<ubyte>& buffer, string name, string& method, Params... params)
    {
        Packet::build(buffer, _magic, _crypt, _key, _rsaKey, name, method, params...);
    }
};

}
}
