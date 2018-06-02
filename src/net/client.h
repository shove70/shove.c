#pragma once

#include <iostream>
#include <vector>

#include "socket.h"

using namespace std;

namespace shove
{
namespace net
{

class Client
{

public:

    unsigned short magic;
    bool keepAlive;

    Client(unsigned short magic, bool isKeepAlive = false);
    Client(const string& host, unsigned short port, unsigned short magic, bool isKeepAlive = false);
    ~Client();
    void setHost(const string& host, unsigned short port);
    int  connect();
    long send(const vector<unsigned char>& sendBuffer);
    long receive(vector<unsigned char>& receiveBuffer);
    void close();
    void reset();
    string& getHost();

protected:

    Socket* socket;
    string host;
    unsigned short port;

private:

    void createSocket();
};

}
}
