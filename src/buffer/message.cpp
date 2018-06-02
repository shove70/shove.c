#include "message.h"

namespace shove
{
namespace buffer
{

ushort       _magic;
CryptType    _crypt;
string       _key;
RSAKeyInfo   _rsaKey;

void Message::settings(ushort magic, CryptType crypt, string key)
{
    assert(crypt == CryptType::NONE || (crypt != CryptType::NONE && !key.empty())); // Must specify key when specifying the type of CryptType.

    _magic = magic;
    _crypt = crypt;
    _key   = key;

    if ((crypt == CryptType::RSA) || (crypt == CryptType::RSA_XTEA_MIXIN))
    {
        _rsaKey = RSA::decodeKey(key);
    }
}

void Message::settings(ushort magic, RSAKeyInfo rsaKey, bool mixinXteaMode)
{
    _magic = magic;
    _crypt = mixinXteaMode ? CryptType::RSA_XTEA_MIXIN : CryptType::RSA;
    _rsaKey = rsaKey;
}

void Message::getMessageInfo(ubyte* buffer, size_t len, string& name, string& method)
{
    Packet::parseInfo(buffer, len, name, method);
}

void Message::deserialize(vector<Any>& result, ubyte* buffer, size_t len, string& name, string& method)
{
    Packet::parse(result, buffer, len, _magic, _crypt, _key, _rsaKey, name, method);
}

}
}
