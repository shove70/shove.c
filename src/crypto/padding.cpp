#include "padding.h"

namespace shove
{
namespace crypto
{

size_t Padding::padding(ubyte* input, size_t len, size_t blockSize, ubyte* output, PaddingMode paddingMode)
{
    switch (paddingMode)
    {
        case PaddingMode::NoPadding:
            return PaddingNoPadding::padding(input, len, blockSize, output);
        case PaddingMode::ANSIX923:
            return PaddingANSIX923::padding(input, len, blockSize, output);
        case PaddingMode::ISO10126:
            return PaddingISO10126::padding(input, len, blockSize, output);
        case PaddingMode::PKCS5:
            return PaddingPKCS5::padding(input, len, blockSize, output);
        case PaddingMode::PKCS7:
            return PaddingPKCS7::padding(input, len, blockSize, output);
        case PaddingMode::Zeros:
            return PaddingZeros::padding(input, len, blockSize, output);
    }

    throw("Error Padding Mode.");
}

size_t Padding::unpadding(ubyte* data, size_t len, size_t blockSize, PaddingMode paddingMode)
{
    switch (paddingMode)
    {
        case PaddingMode::NoPadding:
            return PaddingNoPadding::unpadding(data, len, blockSize);
        case PaddingMode::ANSIX923:
            return PaddingANSIX923::unpadding(data, len, blockSize);
        case PaddingMode::ISO10126:
            return PaddingISO10126::unpadding(data, len, blockSize);
        case PaddingMode::PKCS5:
            return PaddingPKCS5::unpadding(data, len, blockSize);
        case PaddingMode::PKCS7:
            return PaddingPKCS7::unpadding(data, len, blockSize);
        case PaddingMode::Zeros:
            return PaddingZeros::unpadding(data, len, blockSize);
    }

    throw("Error Padding Mode.");
}

}
}
