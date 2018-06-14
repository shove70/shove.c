#pragma once

typedef unsigned char       ubyte;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef long long           int64;
typedef unsigned long long  uint64;
#if (not defined __linux) || (defined __ANDROID__)
typedef unsigned long long  ulong;
#endif
typedef long double         float128;
typedef long double         real;
