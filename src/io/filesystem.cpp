#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <sys/stat.h>
#include <iterator>

#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <libgen.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <unistd.h>
#endif

#ifdef __linux
    #include <string.h>
#endif
#ifdef __APPLE__
    #include <mach-o/dyld.h>
#endif

#include "filesystem.h"

namespace shove
{
namespace io
{

bool fileExists(const string& filename)
{
#ifdef _MSC_VER
    return (_access(filename.c_str(), 0) != -1);
#else
    return (access(filename.c_str(), F_OK) != -1);
#endif
}

size_t fileLength(const string& filename)
{
    assert(fileExists(filename));

    ifstream is;
    is.open(filename, ios::binary);
    is.seekg(0, ios::end);
    size_t len = (size_t)is.tellg();
    is.close();

    return len;
}

int createDirectory(const string& path)
{
#ifdef _WIN32
    int ret = _mkdir(path.c_str());
#else
    int ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

    return ret;
}

int createDirectories(const string& path)
{
    string dir = path;
    if (dir[dir.length() - 1] != '\\' || dir[dir.length() - 1] != '/')
        dir += "/";

    int len = (int)dir.length();
    char tmp[256] = { 0 };

    for (int i = 0; i < len; ++i)
    {
        tmp[i] = dir[i];

        if (tmp[i] == '\\' || tmp[i] == '/')
        {
            if (!fileExists(tmp))
            {
                int ret = createDirectory(tmp);

                if (ret != 0)
                {
                    return ret;
                }
            }
        }
    }

    return 0;
}

int removeDirectory(const string& path)
{
#ifdef _WIN32
    int ret = _rmdir(path.c_str());
#else
    int ret = rmdir(path.c_str());
#endif
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

void copyFile(const string& src, const string& dst)
{
    ifstream ifs(src, ios::binary);
    ofstream ofs(dst, ios::binary);
    ofs << ifs.rdbuf();
}

void copyDirectory(const string& src, const string& dst)
{
    assert(fileExists(src) && isDir(src));
    assert(!fileExists(dst) || isDir(dst));

    if (!fileExists(dst))
    {
        createDirectory(dst);
    }

    vector<string> files;
    getFiles(src, files);
    for (size_t i = 0; i < files.size(); i++)
    {
        string name = buildPath(dst, baseName(files[i]));
        copyFile(files[i], name);
    }

    vector<string> dirs;
    getDirs(src, dirs);
    for (size_t i = 0; i < dirs.size(); i++)
    {
        string name = buildPath(dst, baseName(dirs[i]));
        copyDirectory(dirs[i], name);
    }
}

string buildPath(const string& root, const string& subPath)
{
    if (root.empty() || subPath.empty())
    {
        return root + subPath;
    }

    if (root[root.length() - 1] == '\\' || root[root.length() - 1] == '/' || subPath[0] == '\\' || subPath[0] == '/')
    {
        return root + subPath;
    }

    return root + "/" + subPath;
}

string baseName(const string& path)
{
#ifdef _WIN32
    if (path.length() == 0 || path == "\\" || path == "/")
    {
        return "";
    }

#ifdef _MSC_VER
    char* t = _strdup(path.c_str());
#else
    char* t = strdup(path.c_str());
#endif
    if (t[strlen(t) - 1] == '\\' || t[strlen(t) - 1] == '/')
    {
        t[strlen(t) - 1] = '\0';
    }

    size_t i = strlen(t);
    while (--i >= 0)
    {
        if (t[i] == '\\' || t[i] == '/')
        {
            break;
        }
    }

    string ret(t + i + 1);
    delete t;
    return ret;
#else
    return basename((char*)path.c_str());
#endif
}

string dirName(const string& path)
{
#ifdef _WIN32

#ifdef _MSC_VER
    char* temp = _strdup(path.c_str());
#else
    char* temp = strdup(path.c_str());
#endif

    for (size_t i = path.length(); i > 0; i--)
    {
       if (temp[i] == '/' || temp[i]=='\\')
       {
           temp[i] = '\0';
           break;
       }
    }

    string ret = temp;
    return ret;
#else
    return dirname((char*)path.c_str());
#endif
}

bool isDir(const string& name)
{
    struct stat buf = { 0 };
    stat(name.c_str(), &buf);
    return ((buf.st_mode & S_IFDIR) != 0);
}

bool isFile(const string& name)
{
    struct stat buf = { 0 };
    stat(name.c_str(), &buf);
    return ((buf.st_mode & S_IFREG) != 0);
}

void getFiles(const string& path, vector<string>& files)
{
#ifdef _WIN32
    intptr_t handle;
    _finddata_t fileinfo;

    string dir = path;
    if (dir[dir.length() - 1] != '\\' || dir[dir.length() - 1] != '/')
    {
        dir += "/";
    }
    dir += "*.*";

    handle = _findfirst(dir.c_str(), &fileinfo);
    if (handle == -1)
    {
        return;
    }

    do
    {
        if (!(fileinfo.attrib & _A_SUBDIR))
        {
            files.push_back(buildPath(path, fileinfo.name));
        }
    }
    while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);
#else
    DIR* dir;
    if (!(dir = opendir(path.c_str())))
    {
        assert(false);
    }

    struct dirent* d_ent;
    string temppath = path;
    if (temppath[temppath.length() - 1] != '\\' || temppath[temppath.length() - 1] != '/')
    {
        temppath += "/";
    }

    while ((d_ent = readdir(dir)) != NULL) {
        struct stat file_stat;
        if (strncmp(d_ent->d_name, ".", 1) == 0 || strncmp(d_ent->d_name, "..", 2) == 0)
        {
            continue;
        }

        string name = temppath + d_ent->d_name;
        if (lstat(name.c_str(), &file_stat) < 0)
        {
            assert(false);
        }

        if (!S_ISDIR(file_stat.st_mode))
        {
            files.push_back(name);
        }
    }

    closedir(dir);
#endif
}

void getDirs(const string& path, vector<string>& dirs)
{
#ifdef _WIN32
    intptr_t handle;
    _finddata_t fileinfo;

    string dir = path;
    if (dir[dir.length() - 1] != '\\' || dir[dir.length() - 1] != '/')
    {
        dir += "/";
    }
    dir += "*.*";

    handle = _findfirst(dir.c_str(), &fileinfo);
    if (handle == -1)
    {
        return;
    }

    do {
        if (fileinfo.attrib & _A_SUBDIR)
        {
            if (strcmp(fileinfo.name, ".") && strcmp(fileinfo.name, ".."))
            {
                dirs.push_back(buildPath(path, fileinfo.name));
            }
        }
    }
    while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);
#else
    DIR* dir;
    if (!(dir = opendir(path.c_str())))
    {
        assert(false);
    }

    struct dirent* d_ent;
    string temppath = path;
    if (temppath[temppath.length() - 1] != '\\' || temppath[temppath.length() - 1] != '/')
    {
        temppath += "/";
    }

    while ((d_ent = readdir(dir)) != NULL)
    {
        struct stat file_stat;
        if (strncmp(d_ent->d_name, ".", 1) == 0 || strncmp(d_ent->d_name, "..", 2) == 0)
        {
            continue;
        }

        string name = temppath + d_ent->d_name;
        if (lstat(name.c_str(), &file_stat) < 0)
        {
            assert(false);
        }

        if (S_ISDIR(file_stat.st_mode))
        {
            dirs.push_back(name);
        }
    }

    closedir(dir);
#endif
}

#ifdef _WIN32
#ifdef _MSC_VER
string thisExePath()
{
    TCHAR path[1024];
    memset(path, '\0', 1024);

    GetModuleFileName(NULL, path, 1024);

#ifdef UNICODE
    int len = WideCharToMultiByte(CP_ACP, 0, path, -1, NULL, 0, NULL, NULL);
    char* ansi = new char[len + 1];
    memset(ansi, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, path, -1, ansi, len, NULL, NULL);
    string ret(ansi);
    delete[] ansi;
#else
    string ret(path);
#endif
    return ret;
}
#else
string thisExePath()
{
    char path[1024];
    memset(path, '\0', 1024);

    GetModuleFileName(NULL, path, 1024);

    string ret(path);
    return ret;
}
#endif
#endif
#ifdef __linux
string thisExePath()
{
    char path[1024];
    memset(path, '\0', 1024);

    int len = readlink("/proc/self/exe", path , 1024);
    if (len > 1024 || len < 0)
    {
        return  "";
    }

    string ret = path;
    return ret;
}
#endif
#ifdef __APPLE__
// #if TARGET_OS_IPHONE, #if TARGET_OS_MAC
string thisExePath()
{
    char buf[0];
    uint size = 0;
    int res = _NSGetExecutablePath(buf, &size);

    char* path = (char*)malloc(size + 1);
    path[size] = 0;
    res = _NSGetExecutablePath(path, &size);

    string ret = path;
    free(path);

    return ret;
}
#endif

#ifdef _WIN32
bool isUtf8(const void* data, size_t size)
{
    bool ret = true;
    unsigned char* start = (unsigned char*)data;
    unsigned char* end = (unsigned char*)data + size;

    while (start < end)
    {
        if (*start < 0x80)
            start++;
        else if (*start < (0xC0))
        {
            ret = false;
            break;
        }
        else if (*start < (0xE0))
        {
            if (start >= end - 1)
                break;

            if ((start[1] & (0xC0)) != 0x80)
            {
                ret = false;
                break;
            }

            start += 2;
        }
        else if (*start < (0xF0))
        {
            if (start >= end - 2)
                break;

            if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
            {
                ret = false;
                break;
            }

            start += 3;
        }
        else
        {
            ret = false;
            break;
        }
    }

    return ret;
}

string ansi2utf8(const string& ansi)
{
    if (isUtf8(ansi.c_str(), (long)ansi.length()))
        return ansi;

    int len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), (int)ansi.length(), NULL, 0);

    WCHAR* lpszW = new WCHAR[len];
    wmemset(lpszW, 0, len);
    int afterLen = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), (int)ansi.length(), lpszW, len);
    assert(afterLen == len);

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, lpszW, len, NULL, 0, NULL, NULL);
    assert(utf8_len > 0);

    char* utf8_string = new char[utf8_len + 1];
    memset(utf8_string, 0, utf8_len + 1);
    afterLen = WideCharToMultiByte(CP_UTF8, 0, lpszW, len, utf8_string, utf8_len, NULL, NULL);
    assert(afterLen == utf8_len);

    string ret(utf8_string);

    delete[] lpszW;
    delete[] utf8_string;

    return ret;
}

string utf82ansi(const string& utf8)
{
    if (!isUtf8(utf8.c_str(), (long)utf8.length()))
        return utf8;

    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.length(), NULL, 0);

    WCHAR* lpszW = new WCHAR[len];
    wmemset(lpszW, 0, len);
    int afterLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.length(), lpszW, len);
    assert(afterLen == len);

    int ansi_len = WideCharToMultiByte(CP_ACP, 0, lpszW, len, NULL, 0, NULL, NULL);
    assert(ansi_len > 0);

    char* ansi_string = new char[ansi_len + 1];
    memset(ansi_string, 0, ansi_len + 1);
    afterLen = WideCharToMultiByte(CP_ACP, 0, lpszW, len, ansi_string, ansi_len, NULL, NULL);
    assert(afterLen == ansi_len);

    string ret(ansi_string);

    delete[] lpszW;
    delete[] ansi_string;

    return ret;
}
#endif

}
}
