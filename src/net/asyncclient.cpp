#include <chrono>
#include <thread>
#include <cassert>

#include "asyncclient.h"

namespace shove
{
namespace net
{

AsyncClient::AsyncClient(const string& host, unsigned short port, unsigned short magic, OnReceive onReceive, int receiveTimeout) : Client(host, port, magic, true)
{
    assert(onReceive != NULL);

    this->onReceive = onReceive;
    socket->Connect(host, port);

    thread t(AsyncClient::asyncReceive, this, receiveTimeout);
    t.detach();
}

void AsyncClient::asyncReceive(AsyncClient* client, int receiveTimeout)
{
    while (true)
    {
        if (!client->socket->IsConnected())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            continue;
        }

        vector<unsigned char> receiveBuffer;

        if (client->receive(receiveBuffer, receiveTimeout) > 0)
        {
            client->onReceive(receiveBuffer);
        }
    }
}

}
}
