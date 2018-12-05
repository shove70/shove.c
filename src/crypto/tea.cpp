#include <cassert>

#include "common.h"
#include "tea.h"

namespace shove
{
namespace crypto
{

// TEA
TEA::TEA(int* key)
{
    this->DELTA    = 0x9E3779B9;
    this->m_key    = key;
    this->m_rounds = 32;
}

size_t TEA::padding(ubyte* data, size_t len, ubyte* output)
{
    size_t output_len = len;
    while ((output_len + 4) % 8 != 0) output_len++;

    for (size_t i = 0; i < len; i++)
    {
        output[i] = data[i];
    }

    for (size_t i = len; i < output_len; i++)
    {
        output[i] = 0;
    }

    writeIntToBytes<uint>((uint)len, output + output_len, ENDIAN_BIG);

    return output_len + 4;
}

// Encrypt given ubyte array (length to be crypted must be 8 ubyte aligned)
size_t TEA::encrypt(ubyte* data, size_t len, ubyte* result)
{
    size_t output_len = padding(data, len, result);

    for (size_t i = 0; i < (output_len + 4) / 8; i++)
    {
        int v0 = readIntFromBytes<int>(result + (i * 8));
        int v1 = readIntFromBytes<int>(result + (i * 8 + 4));

        int sum = 0;

        for (int j = 0; j < m_rounds; j++)
        {
            sum += DELTA;
            v0 += ((v1 << 4) + m_key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + m_key[1]);
            v1 += ((v0 << 4) + m_key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + m_key[3]);
        }

        writeIntToBytes<int>(v0, result + (i * 8));
        writeIntToBytes<int>(v1, result + (i * 8 + 4));
    }

    return output_len;
}

// Decrypt given ubyte array (length to be crypted must be 8 ubyte aligned)
size_t TEA::decrypt(ubyte* data, size_t len, ubyte* result)
{
    assert(len > 0 && len % 8 == 0);

    for (size_t i = 0; i < len; i++)
    {
        result[i] = data[i];
    }

    for (size_t i = 0; i < len / 8; i++)
    {
        int v0 = readIntFromBytes<int>(result + (i * 8));
        int v1 = readIntFromBytes<int>(result + (i * 8 + 4));

        int sum = (int)((uint)DELTA * (uint)m_rounds);  //0xC6EF3720

        for (int j = 0; j < m_rounds; j++)
        {
            v1 -= ((v0 << 4) + m_key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + m_key[3]);
            v0 -= ((v1 << 4) + m_key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + m_key[1]);
            sum -= DELTA;
        }

        writeIntToBytes<int>(v0, result + (i * 8));
        writeIntToBytes<int>(v1, result + (i * 8 + 4));
    }

    size_t result_len = readIntFromBytes<uint>(result + (len - 4), ENDIAN_BIG);

    if ((result_len < 0) || (result_len > (len - 4)))
    {
        return 0;
    }

    for (size_t i = result_len; i < (len - 4); i++)
    {
        if (result[i] != 0)
        {
            return 0;
        }
    }

    return result_len;
}

size_t TEAUtils::encrypt(ubyte* data, size_t len, int key[], ubyte* result)
{
    return handle(data, len, key, result, 1);
}

size_t TEAUtils::decrypt(ubyte* data, size_t len, int key[], ubyte* result)
{
    return handle(data, len, key, result, 2);
}

size_t TEAUtils::handle(ubyte* data, size_t len, int key[], ubyte* result, int EorD)
{
    TEA tea(key);
    return (EorD == 1) ? tea.encrypt(data, len, result) : tea.decrypt(data, len, result);
}


// XTEA
XTEA::XTEA(int* key, int rounds)
{
    this->DELTA    = 0x9E3779B9;
    this->m_key    = key;
    this->m_rounds = rounds;
}

size_t XTEA::padding(ubyte* data, size_t len, ubyte* output)
{
    size_t output_len = len;
    while ((output_len + 4) % 8 != 0) output_len++;

    for (size_t i = 0; i < len; i++)
    {
        output[i] = data[i];
    }

    for (size_t i = len; i < output_len; i++)
    {
        output[i] = 0;
    }

    writeIntToBytes<uint>((uint)len, output + output_len, ENDIAN_BIG);

    return output_len + 4;
}

// Encrypt given ubyte array (length to be crypted must be 8 ubyte aligned)
size_t XTEA::encrypt(ubyte* data, size_t len, ubyte* result)
{
    size_t output_len = padding(data, len, result);

    for (size_t i = 0; i < (output_len + 4) / 8; i++)
    {
        int v0 = readIntFromBytes<int>(result + (i * 8));
        int v1 = readIntFromBytes<int>(result + (i * 8 + 4));

        int sum = 0;

        for (int j = 0; j < m_rounds; j++)
        {
            v0 += ((v1 << 4 ^ (int)((uint)v1 >> 5)) + v1) ^ (sum + m_key[sum & 3]);
            sum += DELTA;
            v1 += ((v0 << 4 ^ (int)((uint)v0 >> 5)) + v0) ^ (sum + m_key[(int)((uint)sum >> 11) & 3]);
        }

        writeIntToBytes<int>(v0, result + (i * 8));
        writeIntToBytes<int>(v1, result + (i * 8 + 4));
    }

    return output_len;
}

// Decrypt given ubyte array (length to be crypted must be 8 ubyte aligned)
size_t XTEA::decrypt(ubyte* data, size_t len, ubyte* result)
{
    assert(len > 0 && len % 8 == 0);

    for (size_t i = 0; i < len; i++)
    {
        result[i] = data[i];
    }

    for (size_t i = 0; i < len / 8; i++)
    {
        int v0 = readIntFromBytes<int>(result + (i * 8));
        int v1 = readIntFromBytes<int>(result + (i * 8 + 4));

        int sum = (int)((uint)DELTA * (uint)m_rounds);

        for (int j = 0; j < m_rounds; j++)
        {
            v1 -= ((v0 << 4 ^ (int)((uint)v0 >> 5)) + v0) ^ (sum + m_key[(int)((uint)sum >> 11) & 3]);
            sum -= DELTA;
            v0 -= ((v1 << 4 ^ (int)((uint)v1 >> 5)) + v1) ^ (sum + m_key[sum & 3]);
        }

        writeIntToBytes<int>(v0, result + (i * 8));
        writeIntToBytes<int>(v1, result + (i * 8 + 4));
    }

    size_t result_len = readIntFromBytes<uint>(result + (len - 4), ENDIAN_BIG);

    if ((result_len < 0) || (result_len > (len - 4)))
    {
        return 0;
    }

    for (size_t i = result_len; i < (len - 4); i++)
    {
        if (result[i] != 0)
        {
            return 0;
        }
    }

    return result_len;
}

size_t XTEAUtils::encrypt(ubyte* data, size_t len, int key[], ubyte* result)
{
    return handle(data, len, key, result, 1);
}

size_t XTEAUtils::decrypt(ubyte* data, size_t len, int key[], ubyte* result)
{
    return handle(data, len, key, result, 2);
}

size_t XTEAUtils::handle(ubyte* data, size_t len, int key[], ubyte* result, int EorD)
{
    XTEA xtea(key, 64);
    return (EorD == 1) ? xtea.encrypt(data, len, result) : xtea.decrypt(data, len, result);
}

}
}
