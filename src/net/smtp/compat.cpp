#include <string>
#include <vector>
#include "compat.h"

namespace shove
{
namespace net
{
namespace smtp
{

bool Connect(SOCKET sockfd, const SOCKADDR_IN& addr)
{
#ifdef WIN32
    return bool(connect(sockfd, (sockaddr*)&addr, addr.get_size()) != SOCKET_ERROR);
#else
    return bool(connect(sockfd, (sockaddr*) &addr, addr.get_size()) == 0);
#endif
}

bool Socket(SOCKET& s, int domain, int type, int protocol)
{
    s = socket(AF_INET, type, protocol);
#ifdef WIN32
    return bool(s != INVALID_SOCKET);
#else
    return bool(s != -1);
#endif
}

bool Send(int &CharsSent, SOCKET s, const char *msg, size_t len, int flags)
{
    CharsSent = send(s, msg, len, flags);
#ifdef WIN32
    return bool((CharsSent != SOCKET_ERROR));
#else
    return bool((CharsSent != -1));
#endif
}

bool Recv(int &CharsRecv, SOCKET s, char *buf, size_t len, int flags)
{
    CharsRecv = recv(s, buf, len, flags);
#ifdef WIN32
    return bool((CharsRecv != SOCKET_ERROR));
#else
    return bool((CharsRecv != -1));
#endif
}

void Closesocket(const SOCKET& s)
{
#ifdef WIN32
    closesocket(s);
#else
    close(s);
#endif
}

void initNetworking()
{
#ifdef WIN32
    class socks
    {
    public:
        bool init()
        {

            WORD wVersionRequested;
            WSADATA wsaData;

            wVersionRequested = MAKEWORD( 2, 0 );
            int ret = WSAStartup( wVersionRequested, &wsaData);
            if(ret)
            return false;
            initialised = true;
            return true;
        }
        bool IsInitialised() const
        {   return initialised;}
        socks():initialised(false)
        {   init();}
        ~socks()
        {
            if(initialised)
            shutdown();
        }
    private:
        void shutdown()
        {   WSACleanup();}
        bool initialised;
    };
    static socks s;
#endif
}

}
}
}
