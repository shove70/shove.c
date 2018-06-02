#pragma once

#include "../../container/any.h"

#include "../message.h"

using namespace shove::container;

namespace shove
{
namespace buffer
{
namespace rpc
{

typedef void (*TcpRequestHandler)(vector<ubyte>& send_buffer, vector<ubyte>& receive_buffer);
static TcpRequestHandler Handler = NULL;

class Client
{

public:

    static void bindTcpRequestHandler(TcpRequestHandler handler)
    {
        Handler = handler;
    }

    template <class T, typename... Params>
    static typename enable_if<is_base_of<Message, T>::value, T>::type call(string method, Params... params)
    {
        assert(Handler != NULL);        // TcpRequestHandler must be bound.
        assert(method.length() > 0);    // Paramter method must be set.

        vector<ubyte> request;
        Message::serialize_without_msginfo(request, method, params...);

        vector<ubyte> response;
        Handler(request, response);

        vector<Any> res_params;
        string name;
        string res_method;
        Message::deserialize(res_params, response.data(), response.size(), name, res_method);

        //assert(method == res_method);

        T message;
        message.setValue(res_params);
        return message;
    }

    template <typename T, typename... Params>
    static typename enable_if<is_same<char,                 T>::value ||
                              is_same<unsigned char,        T>::value ||
                              is_same<short,                T>::value ||
                              is_same<unsigned short,       T>::value ||
                              is_same<int,                  T>::value ||
                              is_same<unsigned int,         T>::value ||
                              is_same<long,                 T>::value ||
                              is_same<unsigned long,        T>::value ||
                              is_same<long long,            T>::value ||
                              is_same<unsigned long long,   T>::value ||
                              is_same<float,                T>::value ||
                              is_same<double,               T>::value ||
                              is_same<long double,          T>::value ||
                              is_same<bool,                 T>::value ||
                              is_same<string,               T>::value, T>::type call(string method, Params... params)
    {
        assert(Handler != NULL);        // TcpRequestHandler must be bound.
        assert(method.length() > 0);    // Paramter method must be set.

        vector<ubyte> request;
        Message::serialize_without_msginfo(request, method, params...);

        vector<ubyte> response;
        Handler(request, response);

        vector<Any> res_params;
        string name;
        string res_method;
        Message::deserialize(res_params, response.data(), response.size(), name, res_method);

        //assert(method == res_method);

        return res_params[0].cast<T>();
    }
};

}
}
}
