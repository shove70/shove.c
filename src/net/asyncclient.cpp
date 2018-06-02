#include <chrono>
#include <thread>
#include <cassert>

#include "asyncclient.h"

namespace shove
{
namespace net
{

AsyncClient::AsyncClient(const string& host, unsigned short port, unsigned short magic, OnReceive onReceive) : Client(host, port, magic, true)
{
    assert(onReceive != NULL);

    this->onReceive = onReceive;
    socket->Connect(host, port);

    thread t(AsyncClient::asyncReceive, this);
    t.detach();
}

void AsyncClient::asyncReceive(AsyncClient* client)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!client->socket->IsConnected())
        {
            continue;
        }

        vector<unsigned char> receiveBuffer;

        if (client->receive(receiveBuffer) > 0)
        {
            client->onReceive(receiveBuffer);
        }
    }
}

}
}
