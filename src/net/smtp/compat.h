#pragma once

#ifdef _WIN32
#pragma warning( disable : 4786 )
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#endif

namespace shove
{
namespace net
{
namespace smtp
{

struct SOCKADDR_IN
{
    sockaddr_in ADDR;

    SOCKADDR_IN(const std::string& address, unsigned short port, short family = AF_INET)
    {
        ADDR.sin_port = port;
        ADDR.sin_family = family;
#ifdef WIN32
        ADDR.sin_addr.S_un.S_addr = inet_addr(address.c_str());
        ok = (ADDR.sin_addr.S_un.S_addr != INADDR_NONE);
#else
        ok = (inet_aton(address.c_str(), &ADDR.sin_addr));
#endif
    }

    SOCKADDR_IN(const SOCKADDR_IN& addr)
    {
        ADDR = addr.ADDR;
        ok = addr.ok;
    }
    SOCKADDR_IN operator =(const SOCKADDR_IN& addr)
    {
        ADDR = addr.ADDR;
        ok = addr.ok;
        return *this;
    }

    operator bool() const
    {
        return ok;
    }

    operator const sockaddr_in() const
    {
        return ADDR;
    }

    operator const sockaddr() const
    {
        sockaddr addr;
        std::copy((char*) &ADDR, (char*) &ADDR + sizeof(ADDR), (char*) &addr);
        return addr;
    }

    size_t get_size() const
    {
        return sizeof(ADDR);
    }

    char* get_sin_addr()
    {
        return (char*) &ADDR.sin_addr;
    }
    void set_port(unsigned short newport)
    {
        ADDR.sin_port = newport;
    }
    void set_ip(const std::string& newip)
    {
#ifdef WIN32
        ADDR.sin_addr.S_un.S_addr = inet_addr(newip.c_str());
        ok = (ADDR.sin_addr.S_un.S_addr != INADDR_NONE);
#else
        ok = (inet_aton(newip.c_str(), &ADDR.sin_addr));
#endif
    }
    void zeroaddress()
    {
#ifdef WIN32
        ADDR.sin_addr.S_un.S_addr = 0;
#else
        ADDR.sin_addr.s_addr = 0;
#endif
    }

private:
    bool ok;
};

bool Connect(SOCKET sockfd, const SOCKADDR_IN& addr);
bool Socket(SOCKET& s, int domain, int type, int protocol);
bool Send(int &CharsSent, SOCKET s, const char *msg, size_t len, int flags);
bool Recv(int &CharsRecv, SOCKET s, char *buf, size_t len, int flags);
void Closesocket(const SOCKET& s);
void initNetworking();

}
}
}
