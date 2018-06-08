#include <errno.h>
#include <cassert>
#include <cstring>

#include "socket.h"

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#pragma comment(lib, "ws2_32.lib")
#endif

namespace shove
{
namespace net
{

Socket::Socket(int SOCK_TYPE)
{
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        WSACleanup();
    }
#endif

    timeout              = 30;       // Unit: seconds.
    max_send_buffer_size = 0xFFFF;
    max_recv_buffer_size = 0xFFFF;
    port                 = 0;
    connected            = false;

    this->SOCK_TYPE      = SOCK_TYPE;
    m_sock               = -1;
}

Socket::~Socket()
{
    Close();
#ifdef _WIN32
    WSACleanup();
#endif
}

SOCKET Socket::GetHandle()
{
    return m_sock;
}

void Socket::Close()
{
    if (m_sock != _SOCKET_ERROR)
    {
#ifdef _WIN32
        closesocket(m_sock);
#else
        close(m_sock);
#endif
        m_sock = _SOCKET_ERROR;
    }

    connected = false;
}

void Socket::Shutdown()
{
    if (m_sock != _SOCKET_ERROR)
    {
#ifdef _WIN32
        shutdown(m_sock, SD_BOTH);
#else
        shutdown(m_sock, SHUT_RDWR);
#endif
    }

    connected = false;
}

bool Socket::Connect(const string& host, unsigned short port)
{
    char ip[128];
    memset(ip, 0, sizeof(ip));
    strcpy(ip, host.c_str());

    void*  svraddr     = nullptr;
    int    error       = -1;
    int    svraddr_len = 0;
    bool   ret         = true;
    struct sockaddr_in  svraddr_4;
    struct sockaddr_in6 svraddr_6;

    struct addrinfo *result = NULL;
    error = getaddrinfo(host.c_str(), NULL, NULL, &result);

    if (error || !result)
    {
        return false;
    }

    const struct sockaddr *sa = result->ai_addr;
    socklen_t maxlen = 128;

    switch (sa->sa_family)
    {
    case AF_INET: // ipv4
        if ((m_sock = socket(AF_INET, SOCK_TYPE, 0)) < 0)
        {
            ret = false;
            break;
        }
        if (!inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), ip, maxlen))
        {
            ret = false;
            break;
        }
        svraddr_4.sin_family      = AF_INET;
        svraddr_4.sin_addr.s_addr = inet_addr(ip);
        svraddr_4.sin_port        = htons(port);
        svraddr_len               = sizeof(svraddr_4);
        svraddr                   = &svraddr_4;
        break;
    case AF_INET6: // ipv6
        if ((m_sock = socket(AF_INET6, SOCK_TYPE, 0)) < 0)
        {
            ret = false;
            break;
        }
        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), ip, maxlen);
        memset(&svraddr_6, 0, sizeof(svraddr_6));
        svraddr_6.sin6_family = AF_INET6;
        svraddr_6.sin6_port = htons(port);
        if (inet_pton(AF_INET6, ip, &svraddr_6.sin6_addr) < 0)
        {
            ret = false;
            break;
        }
        svraddr_len = sizeof(svraddr_6);
        svraddr     = &svraddr_6;
        break;
    default:
        ret = false;
    }

    freeaddrinfo(result);

    if (m_sock == _SOCKET_ERROR || !ret)
    {
        return false;
    }

    this->host = host;
    this->port = port;

    setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char*)&max_send_buffer_size, sizeof(max_send_buffer_size));
    setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char*)&max_recv_buffer_size, sizeof(max_recv_buffer_size));

    int on = 1;
#ifdef _WIN32
    setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(on));
    int _timeout = timeout * 1000;
    //setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&_timeout, sizeof(int));
    setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&_timeout, sizeof(int));
#else
    setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
    struct timeval _timeout = { timeout, 0 };
    //setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&_timeout, sizeof(_timeout));
    setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&_timeout, sizeof(_timeout));
#endif
#ifdef __APPLE__
    int set = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
#ifdef __linux
    signal(SIGPIPE, SIG_IGN);
#endif

    if (SOCK_TYPE == SOCK_DGRAM)
    {
        connected = true;
    }
    else if (connect(m_sock, (struct sockaddr*)svraddr, svraddr_len))
    {
        Close();
    }
    else
    {
        connected = true;
    }

    return connected;
}

long Socket::Send(const char* buf, long buflen)
{
    if (m_sock == _SOCKET_ERROR)
    {
        return -1;
    }

    long sent = 0;

    while (sent < buflen)
    {
        long len = (long)send(m_sock, buf + sent, buflen - sent, 0);
        if (len <= 0)
        {
            return len;
        }
        sent += len;
    }

    return sent;
}

long Socket::Recv(char* buf, long buflen, int timeout)
{
    if (m_sock == _SOCKET_ERROR)
    {
        return -1;
    }

    if (timeout == 0)
    {
        timeout = this->timeout;
    }

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(m_sock, &fd);

    long receive = 0;
    while (receive < buflen)
    {
        struct timeval val = { timeout, 0 };
        int selret = select((int)m_sock + 1, &fd, NULL, NULL, (timeout == -1) ? NULL : &val);

        if (selret <= 0)
        {
            return selret;
        }

        long len = (long)recv(m_sock, buf + receive, buflen - receive, 0);
        if (len <= 0)
        {
            return len;
        }
        receive += len;
    }

    return receive;
}

bool Socket::GetPeerName(string& ip, unsigned short &port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int addrlen = sizeof(addr);
#ifdef WIN32
    if(getpeername(m_sock, (struct sockaddr*)&addr, &addrlen)!=0)
#else
    if (getpeername(m_sock, (struct sockaddr*) &addr, (socklen_t*) &addrlen) != 0)
#endif
    {
        return false;
    }

    char szIP[64];
#ifdef WIN32
    sprintf(szIP, "%lu.%lu.%lu.%lu", addr.sin_addr.S_un.S_addr & 0xFF, (addr.sin_addr.S_un.S_addr >> 8) & 0xFF, (addr.sin_addr.S_un.S_addr >> 16) & 0xFF, (addr.sin_addr.S_un.S_addr >> 24) & 0xFF);
#else
    sprintf(szIP, "%u.%u.%u.%u", addr.sin_addr.s_addr & 0xFF, (addr.sin_addr.s_addr >> 8) & 0xFF, (addr.sin_addr.s_addr >> 16) & 0xFF, (addr.sin_addr.s_addr >> 24) & 0xFF);
#endif
    ip = szIP;
    port = ntohs(addr.sin_port);

    return true;
}

bool Socket::IsConnected()
{
    return (m_sock != _SOCKET_ERROR) && connected;
}

int Socket::SocketType()
{
    return SOCK_TYPE;
}

long Socket::SendTo(const char* buf, int len, const struct sockaddr_in* toaddr, int tolen)
{
    if (m_sock == _SOCKET_ERROR)
    {
        return -1;
    }

    return sendto(m_sock, buf, len, 0, (const struct sockaddr*)toaddr, tolen);
}

long Socket::RecvFrom(char* buf, int len, struct sockaddr_in* fromaddr, int* fromlen)
{
    if (m_sock == _SOCKET_ERROR)
    {
        return -1;
    }

#ifdef WIN32
    return recvfrom(m_sock,buf,len,0,(struct sockaddr*)fromaddr,fromlen);
#else
    return recvfrom(m_sock, buf, len, 0, (struct sockaddr*)fromaddr, (socklen_t*)fromlen);
#endif
}

bool Socket::Bind(unsigned short port)
{
    if (m_sock == _SOCKET_ERROR) {
        return false;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
#ifdef WIN32
    sin.sin_addr.S_un.S_addr = 0;
#else
    sin.sin_addr.s_addr = 0;
#endif
    memset(sin.sin_zero, 0, 8);
    sin.sin_port = htons(port);

    if (::bind(m_sock, (sockaddr*)&sin, sizeof(sockaddr_in)) != 0)
    {
        return false;
    }

    listen(m_sock, 1024);

    connected = true;
    return true;
}

bool Socket::Accept(Socket& client)
{
    if (m_sock == _SOCKET_ERROR)
    {
        return false;
    }

    client.m_sock = accept(m_sock, NULL, NULL);
    client.connected = true;

    return (client.m_sock != _SOCKET_ERROR);
}

void Socket::SetKeepAlive()
{
    int keepAlive    = 1;
    setsockopt(m_sock, SOL_SOCKET,  SO_KEEPALIVE,  (const char*)&keepAlive,    sizeof(keepAlive));

#ifdef __linux
    int keepIdle     = 30;
    int keepInterval = 5;
    int keepCount    = 3;

    setsockopt(m_sock, SOL_TCP,     TCP_KEEPIDLE,  (const char*)&keepIdle,     sizeof(keepIdle));
    setsockopt(m_sock, SOL_TCP,     TCP_KEEPINTVL, (const char*)&keepInterval, sizeof(keepInterval));
    setsockopt(m_sock, SOL_TCP,     TCP_KEEPCNT,   (const char*)&keepCount,    sizeof(keepCount));
#endif

#ifdef __APPLE__
    int keepIdle     = 30;
    setsockopt(m_sock, IPPROTO_TCP, TCP_KEEPALIVE, (const char*)&keepIdle,     sizeof(keepIdle));
#endif

#ifdef _WIN32
    int keepIdle     = 30;
    int keepInterval = 5;

    tcp_keepalive opt, opt_out;
    opt.keepaliveinterval = keepInterval;
    opt.keepalivetime = keepIdle;
    opt.onoff = keepAlive;
    DWORD dw;
    WSAIoctl(m_sock, SIO_KEEPALIVE_VALS, &opt, sizeof(opt), &opt_out, sizeof(opt_out), &dw, NULL, NULL); //if fail, return: SOCKET_ERROR
#endif

}

}
}
