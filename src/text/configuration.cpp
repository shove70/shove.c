#include "configuration.h"

namespace shove
{

Configuration::Configuration(const string& filename)
{
    FILE* fp;

    char buf[4096];

    if ((fp = fopen(filename.c_str(), "r")) == NULL)
    {
        if ((fp = fopen(filename.c_str(), "w")) == NULL)
        {
            return;
        }

        fclose(fp);

        if ((fp = fopen(filename.c_str(), "r")) == NULL)
        {
            return;
        }
    }

    while (fgets(buf, 4096, fp) != NULL)
    {
        string line = buf;
        int locate = (int)line.find_first_of("=");

        if (locate < 0)
        {
            continue;
        }

        string key = trim_all(line.substr(0, locate));
        string value = trim_all(line.substr(locate + 1, line.length() - locate - 1));
        data[key] = value;
    }

    fclose(fp);
}

string Configuration::trim_all(const string& input)
{
    string space[7] = {" ", "　", "\t", "\v", "\r\n", "\r", "\n"};
    string ret = input;

    for (int i = 0; i < 7; i++)
    {
        ret = ret.erase(0, ret.find_first_not_of(space[i]));
        ret = ret.erase(ret.find_last_not_of(space[i]) + 1);
    }

    return ret;
}

}
