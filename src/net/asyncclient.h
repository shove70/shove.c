#pragma once

#include <iostream>
#include <vector>

#include "client.h"

using namespace std;
using namespace shove::net;

namespace shove
{
namespace net
{

typedef void (*OnReceive)(vector<unsigned char>& buffer);

class AsyncClient : public Client
{
public:

    OnReceive onReceive;

    AsyncClient(const string& host, unsigned short port, unsigned short magic, OnReceive onReceive, int receiveTimeout = 0);

private:

    static void asyncReceive(AsyncClient* client, int receiveTimeout);
};

}
}
