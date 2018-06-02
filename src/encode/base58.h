#pragma once

#include "../math/bigint.h"
#include "../utils/types.h"

using namespace std;
using namespace shove::math::bigint;

namespace shove
{
namespace encode
{

class Base58
{
private:
    static void init(int*, char*);
    static BigInt decodeToBigInteger(string input);
    static ubyte divmod(ubyte* number, size_t len, int firstDigit, int base, int divisor);
public:
    static string encode(ubyte*, size_t);
    static size_t decode(string const&, ubyte*);
};

}
}
