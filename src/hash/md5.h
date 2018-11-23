#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

using namespace std;

namespace shove
{
namespace hash
{

class MD5
{
private:

    #define __uint8  unsigned char
    #define __uint32 unsigned int

    struct md5_context
    {
        __uint32 total[2];
        __uint32 state[4];
        __uint8  buffer[64];
    };

    void md5_starts(struct md5_context* ctx);
    void md5_process(struct md5_context* ctx, __uint8 data[64]);
    void md5_update(struct md5_context* ctx, __uint8* input, size_t length);
    void md5_finish(struct md5_context* ctx, __uint8 digest[16]);

    string toString(unsigned int* data);

public:

    MD5();
    string GenerateMD5(unsigned char* buffer, size_t length);
};

static MD5 MD5Utils;

}
}
