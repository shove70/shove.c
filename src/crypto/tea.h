#pragma once

#include <stddef.h>

#include "../utils/types.h"

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

    size_t padding(ubyte*, size_t, ubyte*);

public:

    TEA(int*);

    size_t encrypt(ubyte*, size_t, ubyte*);
    size_t decrypt(ubyte*, size_t, ubyte*);
};

class TEAUtils
{
public:

    static size_t encrypt(ubyte* data, size_t len, int key[], ubyte* result);
    static size_t decrypt(ubyte* data, size_t len, int key[], ubyte* result);

private:

    static size_t handle(ubyte* data, size_t len, int key[], ubyte* result, int EorD);
};


// XTEA
class XTEA
{
private:

    int  DELTA;      // XTEA delta constant
    int* m_key;      // Key - 4 integer
    int  m_rounds;   // Round to go - 64 are commonly used

    size_t padding(ubyte*, size_t, ubyte*);

public:

    XTEA(int*, int);

    size_t encrypt(ubyte*, size_t, ubyte*);
    size_t decrypt(ubyte*, size_t, ubyte*);
};

class XTEAUtils
{
public:

    static size_t encrypt(ubyte* data, size_t len, int key[], ubyte* result);
    static size_t decrypt(ubyte* data, size_t len, int key[], ubyte* result);

private:

    static size_t handle(ubyte* data, size_t len, int key[], ubyte* result, int EorD);
};

}
}
