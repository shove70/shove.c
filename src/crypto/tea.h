#pragma once

#include <stddef.h>

#include "../utils/types.h"

#include "padding.h"

namespace shove
{
namespace crypto
{

// TEA
class TEA
{
private:

    int  DELTA;      // XTEA delta constant
    int* m_key;      // Key - 4 integer
    int  m_rounds;   // Round: 32

public:

    TEA(int*);

    size_t encrypt(ubyte*, size_t, ubyte*, PaddingMode paddingMode = PaddingMode::NoPadding);
    size_t decrypt(ubyte*, size_t, ubyte*, PaddingMode paddingMode = PaddingMode::NoPadding);
};

class TEAUtils
{
public:

    static size_t encrypt(ubyte* data, size_t len, int key[], ubyte* result, PaddingMode paddingMode = PaddingMode::NoPadding);
    static size_t decrypt(ubyte* data, size_t len, int key[], ubyte* result, PaddingMode paddingMode = PaddingMode::NoPadding);

private:

    static size_t handle(ubyte* data, size_t len, int key[], ubyte* result, int EorD, PaddingMode paddingMode);
};


// XTEA
class XTEA
{
private:

    int  DELTA;      // XTEA delta constant
    int* m_key;      // Key - 4 integer
    int  m_rounds;   // Round to go - 64 are commonly used

public:

    XTEA(int*, int);

    size_t encrypt(ubyte*, size_t, ubyte*, PaddingMode paddingMode = PaddingMode::NoPadding);
    size_t decrypt(ubyte*, size_t, ubyte*, PaddingMode paddingMode = PaddingMode::NoPadding);
};

class XTEAUtils
{
public:

    static size_t encrypt(ubyte* data, size_t len, int key[], ubyte* result, PaddingMode paddingMode = PaddingMode::NoPadding);
    static size_t decrypt(ubyte* data, size_t len, int key[], ubyte* result, PaddingMode paddingMode = PaddingMode::NoPadding);

private:

    static size_t handle(ubyte* data, size_t len, int key[], ubyte* result, int EorD, PaddingMode paddingMode);
};

}
}
