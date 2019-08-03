#include <cassert>

#include "client.h"
#include "../utils/bitmanip.h"

using namespace shove::utils;

namespace shove
{
namespace net
{

Client::Client(unsigned short magic, bool isKeepAlive)
{
    socket = NULL;
    port   = 0;

    this->magic     = magic;
    this->keepAlive = isKeepAlive;
}

Client::Client(const string& host, unsigned short port, unsigned short magic, bool isKeepAlive)
{
    socket = NULL;

    this->host      = host;
    this->port      = port;
    this->magic     = magic;
    this->keepAlive = isKeepAlive;
}

Client::~Client()
{
    if (!socket)
    {
        return;
    }

    if (socket->IsConnected())
    {
        socket->Shutdown();
        socket->Close();
    }

    delete socket;
    socket = NULL;
}

void Client::setHost(const string& host, unsigned short port)
{
    this->host = host;
    this->port = port;
}

int Client::connect()
{
    assert(!host.empty());

    if (!socket)
    {
        createSocket();
    }

    if (!socket->IsConnected())
    {
        if (!socket->Connect(host, port))
        {
            return -1;
        }
    }

    return 0;
}

void Client::setReceiveTimeout(int seconds)
{
    socket->timeout = seconds;
}

long Client::send(const vector<unsigned char>& sendBuffer)
{
    if (!socket->IsConnected())
    {
        if (!socket->Connect(host, port))
        {
            return -1;
        }
    }

    if (sendBuffer.size() == 0)
    {
        return -2;
    }

    long len = socket->Send((char*)(sendBuffer.data()), (long)(sendBuffer.size()));

    if (len != (long)(sendBuffer.size()))
    {
        return -3;
    }

    return len;
}

long Client::receive(vector<unsigned char>& receiveBuffer, int timeout)
{
    if (!socket->IsConnected())
    {
        return -1;
    }

    unsigned char buffer[6];
    long len;
    if ((len = socket->Recv((char*)buffer, 2, timeout)) != 2)
    {
        return len;
    }
    if (magic != Bitmanip::peek<unsigned short>(buffer, 0))
    {
        return -2;
    }

    if ((len = socket->Recv((char*)buffer + 2, 4, timeout)) != 4)
    {
        return len;
    }
    int size = Bitmanip::peek<int>(buffer, 2);
    if (size <= 0)
    {
        return -3;
    }

    unsigned char* data = new unsigned char[size];
    len = socket->Recv((char*)data, size, timeout);

    if (len != (long)size)
    {
        delete[] data;

        return -4;
    }

    receiveBuffer.clear();
    receiveBuffer.reserve(6 + size);
    receiveBuffer.insert(receiveBuffer.end(), buffer, buffer + 6);
    receiveBuffer.insert(receiveBuffer.end(), data, data + size);

    delete[] data;

    return 6 + size;
}

void Client::close()
{
    if (socket == NULL)
    {
        return;
    }

    if (socket->IsConnected())
    {
        socket->Shutdown();
    }

    socket->Close();

    delete socket;
    socket = NULL;
}

void Client::reset()
{
    delete socket;
    socket = NULL;
}

string& Client::getHost()
{
    return host;
}

void Client::createSocket()
{
    assert(socket == NULL);

    socket = new Socket();

    if (keepAlive)
    {
        //socket->SetKeepAlive();
    }
}

}
}
