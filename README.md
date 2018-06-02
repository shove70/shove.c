# C++ Ext Library: practical protocol buffer, RPC, encryption, encode, hash, net, math...

Companies and individuals used in the project's C++ extension library have been tested by actual projects and can be used normally.

# A C++ Library of encryption, decryption, encode, hash, and message digital signatures.

The same functional and fully compatible dlang project:
https://github.com/shove70/crypto

For more examples, see dlang project, Thanks.

# A simple and practical protocol buffer & RPC library.

At present, it has only dlang and C++ implementations, and will add more language support in the future, such as: Lua, Java, python, ruby, golang, rust...

dlang project on github: 
https://github.com/shove70/buffer


### Quick start the buffer:

```
    Sample sample;
    sample.id = 1;
    sample.name = "abcde";
    sample.age = 10;

    vector<ubyte> serialized;
    sample.serialize(serialized);

    Sample sample2 = Message::deserialize<Sample>(serialized.data(), serialized.size());
    cout << sample2.id << ", " << sample2.name << ", " << sample2.age << endl;

    //////////////////////

    Client::bindTcpRequestHandler(&tcpRequestHandler);
    int r = Client::call<int>("Login", 101, "abcde");
    cout << r << endl;

    Sample sample3 = Client::call<Sample>("Login", 1, "abcde", 10);
    cout << sample3.id << ", " << sample3.name << ", " << sample3.age << endl;
    
```
