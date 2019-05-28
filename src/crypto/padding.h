#pragma once

#include <cassert>
#include <string>

#include "../utils/types.h"
#include "../utils/random.h"
#include "../utils/utils.h"
#include "common.h"

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
    Zeros,           // 00 00 00 00 (All zero)
    Customized       // 00 00 00 00 + (00 00 00 04) (Zero + Original size)
};

enum PaddingStuff
{
    PaddingStuff_None, PaddingStuff_Zero, PaddingStuff_Random, PaddingStuff_Size, PaddingStuff_OriginalSize
};

template<PaddingStuff fill, PaddingStuff suffix>
class PaddingImpl
{
public:

    static size_t padding(ubyte* input, size_t len, size_t blockSize, ubyte* output)
    {
        if (!((blockSize > 0) && (blockSize % 8 == 0)))
        {
            throw("Invalid block size, which must be a multiple of 8.");
        }

        if (!((suffix != PaddingStuff_OriginalSize) || (fill == PaddingStuff_Zero)))
        {
            throw("PaddingCustomized require: Zero + OriginalSize.");
        }

        for (size_t i = 0; i < len; i++)
        {
            output[i] = input[i];
        }

        if ((fill == PaddingStuff_None) || (suffix == PaddingStuff_None))
        {
            if (!((len > 0) && (len % blockSize == 0)))
            {
                throw("Invalid data size, which must be a multiple of blockSize.");
            }

            return len;
        }
        else if (suffix != PaddingStuff_OriginalSize)
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
        else
        {
            size_t output_len = len;
            while ((output_len + 4) % 8 != 0) output_len++;

            for (size_t i = len; i < output_len; i++)
            {
                output[i] = 0;
            }

            writeIntToBytes<uint>((uint)len, output + output_len, ENDIAN_BIG);

            return output_len + 4;
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

        if (!((suffix != PaddingStuff_OriginalSize) || (fill == PaddingStuff_Zero)))
        {
            throw("PaddingCustomized require: Zero + OriginalSize.");
        }

        if ((fill == PaddingStuff_None) || (suffix == PaddingStuff_None))
        {
            return len;
        }
        else if ((fill == PaddingStuff_Zero) && (suffix == PaddingStuff_Size))
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
        else if ((fill == PaddingStuff_Random) && (suffix == PaddingStuff_Size))
        {
            size_t size = data[len - 1];

            if (!(size <= blockSize))
            {
                throw("Error Padding Mode.");
            }

            return len - size;
        }
        else if ((fill == PaddingStuff_Size) && (suffix == PaddingStuff_Size))
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
        else if ((fill == PaddingStuff_Zero) && (suffix == PaddingStuff_Zero))
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
        else if ((fill == PaddingStuff_Zero) && (suffix == PaddingStuff_OriginalSize))
        {
            size_t result_len = readIntFromBytes<uint>(data + (len - 4), ENDIAN_BIG);

            if ((result_len <= 0) || (result_len > (len - 4)))
            {
                return 0;
            }

            for (size_t i = result_len; i < (len - 4); i++)
            {
                if (data[i] != 0)
                {
                    return 0;
                }
            }

            return result_len;
        }

        throw("Error Padding Mode.");
    }

private:

    static void fillA(PaddingStuff type, ubyte* data, size_t index, size_t paddingSize)
    {
        switch (type)
        {
            case PaddingStuff_Zero:
                data[index] = 0x00;
                break;
            case PaddingStuff_Random:
                data[index] = (ubyte)rnd.next();
                break;
            case PaddingStuff_Size:
                data[index] = (ubyte)paddingSize;
                break;
            default:
                assert(0);
        }
    }
};

typedef PaddingImpl<PaddingStuff_None,   PaddingStuff_None>         PaddingNoPadding;
typedef PaddingImpl<PaddingStuff_Zero,   PaddingStuff_Size>         PaddingANSIX923;
typedef PaddingImpl<PaddingStuff_Random, PaddingStuff_Size>         PaddingISO10126;
typedef PaddingImpl<PaddingStuff_Size,   PaddingStuff_Size>         PaddingPKCS5;
typedef PaddingImpl<PaddingStuff_Size,   PaddingStuff_Size>         PaddingPKCS7;
typedef PaddingImpl<PaddingStuff_Zero,   PaddingStuff_Zero>         PaddingZeros;
typedef PaddingImpl<PaddingStuff_Zero,   PaddingStuff_OriginalSize> PaddingCustomized;   // For downward compatibility.

class Padding
{
public:

    static size_t padding(ubyte* input, size_t len, size_t blockSize, ubyte* output, PaddingMode paddingMode);
    static size_t unpadding(ubyte* data, size_t len, size_t blockSize, PaddingMode paddingMode);
};

}
}
