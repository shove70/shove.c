#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <mstcpip.h>
    #include <ws2tcpip.h>

    #define _SOCKET_ERROR (SOCKET)SOCKET_ERROR
#else
    #include <unistd.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>

    #define SOCKET int
    #define _SOCKET_ERROR -1
#endif

#include <string>

using namespace std;

namespace shove
{
namespace net
{

class Socket
{
public:
    int timeout;
    unsigned int max_send_buffer_size;
    unsigned int max_recv_buffer_size;

    SOCKET m_sock;

    Socket(int SOCK_TYPE = SOCK_STREAM);    // SOCK_DGRAM
    virtual ~Socket();

    virtual bool Connect(const string& host, unsigned short port);
    virtual bool IsConnected();
    virtual int  SocketType();
    virtual bool Bind(unsigned short port);
    virtual bool Accept(Socket& client);
    virtual void Close();
    virtual void Shutdown();

    virtual long Send(const char* buf, long buflen);
    virtual long Recv(char* buf, long buflen);
    virtual long SendTo(const char* buf, int len, const struct sockaddr_in* toaddr, int tolen);
    virtual long RecvFrom(char* buf, int len, struct sockaddr_in* fromaddr, int* fromlen);

    virtual bool GetPeerName(string& ip, unsigned short &port);
    virtual void SetKeepAlive();

    SOCKET GetHandle();
private:
    int SOCK_TYPE;
    bool connected;

    string host;
    unsigned short port;
};

}
}
