#pragma once

#include <vector>
#include <string>

using namespace std;

namespace shove
{
namespace compress
{

#define PUT_DIR_T   1
#define PUT_FILE_T  2

class Szip
{

public:

    static int compressBytes  (unsigned char* input, size_t len, vector<unsigned char>& output);
    static int uncompressBytes(unsigned char* input, size_t len, vector<unsigned char>& output);
    static int zip            (const string& sourceDirOrFileName, const string& outputFilename);
    static int unzip          (const string& szipFilename, const string& outputPath);

private:

    static void getFolderSize(const string& dir, const string& rootDir, size_t& size);
    static void readFile(const string& dir, const string& rootDir, unsigned char* buffer, size_t& pos);
    static void put(int type, const string& name, unsigned char* buffer, size_t& pos);
};

}
}
