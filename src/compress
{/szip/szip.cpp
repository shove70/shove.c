#include "../szip/szip.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cstring>

#include <zlib.h>

#include "../../utils/bitmanip.h"
#include "../../io/filesystem.h"

#ifdef _MSC_VER
    #pragma comment(lib, "zlib.lib")
#endif

using namespace shove::io;
using namespace shove::utils;

namespace shove
{
namespace compress
{

int Szip::compressBytes(unsigned char* input, size_t len, vector<unsigned char>& output)
{
    unsigned long output_len = compressBound((unsigned long)len);
    unsigned char* buffer = new unsigned char[output_len];

    int result = compress(buffer, &output_len, input, (unsigned long)len);
    if (result != Z_OK)
    {
        delete[] buffer;

        return result;
    }

    output.reserve(output_len);
    output.insert(output.end(), buffer, buffer + output_len);
    delete[] buffer;

    return Z_OK;
}

int Szip::uncompressBytes(unsigned char* input, size_t len, vector<unsigned char>& output)
{
    if (len <= 0)
    {
        return 0;
    }

    unsigned long output_len = (unsigned long)len * 10;
    unsigned char* buffer = new unsigned char[output_len];

    int result = uncompress(buffer, &output_len, input, (unsigned long)len);

    if (result == Z_DATA_ERROR)
    {
        delete[] buffer;

        return result;
    }

    while (result == Z_BUF_ERROR)
    {
        output_len *= 10;
        delete[] buffer;
        buffer = new unsigned char[output_len];

        result = uncompress(buffer, &output_len, input, (unsigned long)len);
    }

    if (result != Z_OK)
    {
        delete[] buffer;

        return result;
    }

    output.reserve(output_len);
    output.insert(output.end(), buffer, buffer + output_len);
    delete[] buffer;

    return Z_OK;
}

int Szip::zip(const string& sourceDirOrFileName, const string& outputFilename)
{
    assert(fileExists(sourceDirOrFileName));
    assert(outputFilename != "");

    size_t srcSize = 0, pos = 0;
    if (isFile(sourceDirOrFileName))
    {
        srcSize = fileLength(sourceDirOrFileName);
    }
    else
    {
        getFolderSize(sourceDirOrFileName, "", srcSize);
    }

    srcSize = (size_t)((srcSize + 1024) * 1.2);
    unsigned char* buffer = new unsigned char[srcSize];
    unsigned char const magic[] = { 12, 29 };

    if (isFile(sourceDirOrFileName))
    {
        put(PUT_FILE_T, sourceDirOrFileName, buffer, pos);
    }
    else
    {
        readFile(sourceDirOrFileName, "", buffer, pos);
    }

    unsigned long output_len = compressBound((unsigned long)pos);
    unsigned char* compressed = new unsigned char[output_len];

    int result = compress(compressed, &output_len, buffer, (unsigned long)pos);
    if (result != Z_OK)
    {
        delete[] buffer;
        delete[] compressed;

        return result;
    }

    delete[] buffer;

    remove(outputFilename.c_str());
    ofstream os;
    os.open(outputFilename, ios::out | ios::binary | ios::app);
    os.write((char*)magic, 2);
    os.write((char*)compressed, output_len);
    os.close();
    delete[] compressed;

    return Z_OK;
}


int Szip::unzip(const string& szipFilename, const string& outputPath)
{
    assert(fileExists(szipFilename));
    size_t len = fileLength(szipFilename);
    assert(len > 2);

    unsigned char const magic[] = { 12, 29 };
    unsigned char* data = new unsigned char[len];
    ifstream fin(szipFilename, ios::binary);
    fin.read((char *)data, len);

    assert(data[0] == magic[0] && data[1] == magic[1]);

    unsigned long output_len = (unsigned long)len * 10;
    unsigned char* buffer = new unsigned char[output_len];
    int result = uncompress(buffer, &output_len, data + 2, (unsigned long)len - 2);

    if (result == Z_DATA_ERROR)
    {
        delete[] data;
        delete[] buffer;

        return result;
    }

    while (result == Z_BUF_ERROR)
    {
        output_len *= 10;
        delete[] buffer;
        buffer = new unsigned char[output_len];

        result = uncompress(buffer, &output_len, data + 2, (unsigned long)len - 2);
    }

    delete[] data;

    if (result != Z_OK)
    {
        delete[] buffer;

        return result;
    }

    if (!fileExists(outputPath))
    {
        createDirectories(outputPath);
    }

    string dir = outputPath;
    size_t pos = 0;
    while (pos < output_len)
    {
        unsigned char type = buffer[pos++];
        unsigned short len = Bitmanip::peek<unsigned short>(buffer, pos);
        pos += 2;
#ifdef _WIN32
        string name = utf82ansi(string((char*)buffer + pos, len));
#else
        string name((char*)buffer + pos, len);
#endif

        pos += len;

        if (type == 0x01)
        {
            dir = buildPath(outputPath, name);
            if (!fileExists(dir))  createDirectories(dir);
        }
        else
        {
            string filename = buildPath(dir, name);
            size_t file_len = Bitmanip::peek<unsigned int>(buffer, pos);
            pos += 4;

            ofstream fout;
            fout.open(filename, ios::binary);
            for (size_t i = 0; i < file_len; i++)
            {
                fout << buffer[i + pos];
            }
            fout << flush;
            fout.close();
            pos += file_len;
        }
    }

    delete[] buffer;
    return Z_OK;
}

// private:

void Szip::getFolderSize(const string& dir, const string& rootDir, size_t& size)
{
    vector<string> files;
    getFiles(dir, files);
    for (size_t i = 0; i < files.size(); i++)
    {
        size += fileLength(files[i]);
    }

    vector<string> dirs;
    getDirs(dir, dirs);
    for (size_t i = 0; i < dirs.size(); i++)
    {
        string t = buildPath(rootDir, baseName(dirs[i]));
        getFolderSize(dirs[i], t, size);
    }
}

void Szip::readFile(const string& dir, const string& rootDir, unsigned char* buffer, size_t& pos)
{
    vector<string> files;
    getFiles(dir, files);
    for (size_t i = 0; i < files.size(); i++)
    {
        put(PUT_FILE_T, files[i], buffer, pos);
    }

    vector<string> dirs;
    getDirs(dir, dirs);
    for (size_t i = 0; i < dirs.size(); i++)
    {
        string t = buildPath(rootDir, baseName(dirs[i]));
        put(PUT_DIR_T, t, buffer, pos);
        readFile(dirs[i], t, buffer, pos);
    }
}

void Szip::put(int type, const string& name, unsigned char* buffer, size_t& pos)
{
    assert(type == PUT_DIR_T || type == PUT_FILE_T);

    buffer[pos++] = (unsigned char)type;
#ifdef _WIN32
    string t = ansi2utf8((type == PUT_FILE_T) ? baseName(name) : name);
#else
    string t = (type == PUT_FILE_T) ? baseName(name) : name;
#endif
    Bitmanip::write<unsigned short>((unsigned short)t.length(), buffer, pos);
    pos += sizeof(unsigned short);
    memcpy(buffer + pos, (unsigned char*)(t.c_str()), t.length());
    pos += t.length();

    if (type == PUT_FILE_T)
    {
        ifstream is;
        is.open(name, ios::binary);
        is.seekg(0, ios::end);
        int len = (int)is.tellg();
        Bitmanip::write<unsigned int>((unsigned int)len, buffer, pos);
        pos += sizeof(unsigned int);
        is.seekg(0, ios::beg);
        is.read((char*)buffer + pos, len);
        pos += len;
        is.close();
    }
}

}
}
