#pragma once

#include <cassert>
#include <string>

#include "../utils/types.h"
#include "../utils/random.h"
#include "../utils/utils.h"

using namespace std;
using namespace shove::utils;

namespace shove
{
namespace crypto
{

enum PaddingMode
{
    NoPadding,       // None
    ANSIX923,        // 00 00 00 04 (Zero   + size)
    ISO10126,        // 0A EB 02 04 (Random + size)
    PKCS5,           // 04 04 04 04 (All size)
    PKCS7,           // 04 04 04 04 (All size)
    Zeros            // 00 00 00 00 (All zero)
};

enum PaddingMode_Type
{
    PaddingMode_Type_None, PaddingMode_Type_Zero, PaddingMode_Type_Random, PaddingMode_Type_Size
};

template<PaddingMode_Type fill, PaddingMode_Type suffix>
class PaddingImpl
{
public:

    static size_t padding(ubyte* input, size_t len, size_t blockSize, ubyte* output)
    {
        if (!((blockSize > 0) && (blockSize % 8 == 0)))
        {
            throw("Invalid block size, which must be a multiple of 8.");
        }

        for (size_t i = 0; i < len; i++)
        {
            output[i] = input[i];
        }

        if ((fill == PaddingMode_Type_None) || (suffix == PaddingMode_Type_None))
        {
            if (!((len > 0) && (len % blockSize == 0)))
            {
                throw("Invalid data size, which must be a multiple of blockSize.");
            }

            return len;
        }
        else
        {
            size_t paddingSize = blockSize - len % blockSize;
            int index = (int)paddingSize - 1;

            fillA(suffix, output, len + index, paddingSize);

            while (--index >= 0)
            {
                fillA(fill, output, len + index, paddingSize);
            }

            return len + paddingSize;
        }
    }

    static size_t unpadding(ubyte* data, size_t len, size_t blockSize)
    {
        if (!((blockSize > 0) && (blockSize % 8 == 0)))
        {
            throw("Invalid block size, which must be a multiple of 8.");
        }

        if (!((len > 0) && (len % blockSize == 0)))
        {
            throw("Invalid data size, which must be a multiple of blockSize.");
        }

        if ((fill == PaddingMode_Type_None) || (suffix == PaddingMode_Type_None))
        {
            return len;
        }
        else if ((fill == PaddingMode_Type_Zero) && (suffix == PaddingMode_Type_Size))
        {
            size_t size = data[len - 1];

            if (!(size <= blockSize))
            {
                throw("Error Padding Mode.");
            }

            for (size_t i = len - size; i < len - 1; i++)
            {
                if (!(data[i] == 0))
                {
                    throw("Error Padding Mode.");
                }
            }

            return len - size;
        }
        else if ((fill == PaddingMode_Type_Random) && (suffix == PaddingMode_Type_Size))
        {
            size_t size = data[len - 1];

            if (!(size <= blockSize))
            {
                throw("Error Padding Mode.");
            }

            return len - size;
        }
        else if ((fill == PaddingMode_Type_Size) && (suffix == PaddingMode_Type_Size))
        {
            size_t size = data[len - 1];

            if (!(size <= blockSize))
            {
                throw("Error Padding Mode.");
            }

            for (size_t i = len - size; i < len - 1; i++)
            {
                if (!(data[i] == size))
                {
                    throw("Error Padding Mode.");
                }
            }

            return len - size;
        }
        else if ((fill == PaddingMode_Type_Zero) && (suffix == PaddingMode_Type_Zero))
        {
            if (!(data[len - 1] == 0))
            {
                throw("Error Padding Mode.");
            }

            int index = (int)len - 1;

            while ((index >= 0) && (data[index] == 0))
            {
                index--;
            }

            return index + 1;
        }

        throw("Error Padding Mode.");
    }

private:

    static void fillA(PaddingMode_Type type, ubyte* data, size_t index, size_t paddingSize)
    {
        switch (type)
        {
            case PaddingMode_Type_Zero:
                data[index] = 0x00;
                break;
            case PaddingMode_Type_Random:
                data[index] = (ubyte)rnd.next();
                break;
            case PaddingMode_Type_Size:
                data[index] = (ubyte)paddingSize;
                break;
            default:
                assert(0);
        }
    }
};

typedef PaddingImpl<PaddingMode_Type_None,   PaddingMode_Type_None> PaddingNoPadding;
typedef PaddingImpl<PaddingMode_Type_Zero,   PaddingMode_Type_Size> PaddingANSIX923;
typedef PaddingImpl<PaddingMode_Type_Random, PaddingMode_Type_Size> PaddingISO10126;
typedef PaddingImpl<PaddingMode_Type_Size,   PaddingMode_Type_Size> PaddingPKCS5;
typedef PaddingImpl<PaddingMode_Type_Size,   PaddingMode_Type_Size> PaddingPKCS7;
typedef PaddingImpl<PaddingMode_Type_Zero,   PaddingMode_Type_Zero> PaddingZeros;

class Padding
{
public:

    static size_t padding(ubyte* input, size_t len, size_t blockSize, ubyte* output, PaddingMode paddingMode);
    static size_t unpadding(ubyte* data, size_t len, size_t blockSize, PaddingMode paddingMode);
};

}
}
