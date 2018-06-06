#include "pinyin.h"

namespace shove
{
namespace convert
{

static const size_t SIZE_ARRAY = sizeof(code_pin) / sizeof(short);

const char* get_pinyin(unsigned short char_zh)
{
    size_t low = 0, high = SIZE_ARRAY - 1;
    size_t index;

    while (high - low != 1)
    {
        index = (low + high) / 2;

        if (code_pin[index] == char_zh)
        {
            return str_pin[index];
        }

        if (code_pin[index] < char_zh)
        {
            low = index;
        }
        else
        {
            high = index;
        }
    }

    return str_pin[code_pin[high] <= char_zh ? high : low];
}

string chineseToPinyin(string const &input, bool isLetteryEnd)
{
    string result;
    unsigned short char_zh;
    size_t inputLength = input.length();
    unsigned char high, low;

    for (size_t i = 0; i < inputLength; ++i)
    {
        high = input[i];

        if (high < 0x80)
        {
            if (isLetteryEnd && (i > 0 && input[i - 1] < 0))
            {
                result.append(1, ' ');
            }

            result.append(1, high);
        }
        else
        {
            if (isLetteryEnd && (i > 0))
            {
                result.append(1, ' ');
            }

            low = input[++i];
            char_zh = (high << 8) + low;
            result.append(get_pinyin(char_zh));
        }
    }

    return result;
}

}
}
