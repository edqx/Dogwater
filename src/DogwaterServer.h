#include <iostream>
#include <winsock2.h>
#include <unordered_map>
#include <thread>

#include "util/MessageReader.h"
#include "util/MessageWriter.h"
#include "Connection.h"
#include "PluginManager.h"

#include "enums.h"

#ifndef DOGWATER_DOGWATERSERVER_H
#define DOGWATER_DOGWATERSERVER_H

#pragma warning(disable:4996)
#define BUFLEN 1024

template<>
struct std::hash<const char*>
{
    std::size_t operator()(const char* const& p) const noexcept
    {
        size_t result = 0;
        size_t s = strlen(p);
        const size_t prime = 31;
        for (size_t i = 0; i < s; ++i) {
            result = p[i] + (result * prime);
        }
        return result;
    }
};

class DogwaterServer {
    SOCKET ServerSocket;

    bool IsListening;
    unsigned int ClientId;
    std::unordered_map<const char*, Connection*> ConnectionMap;

    void GetAddressString(const char* out, sockaddr_in* socket);
    Connection* GetConnectionByAddress(sockaddr_in* socket);
    Connection* GetConnectionByAddress(const char* address);

    void RemoveConnection(Connection* connection);
public:
    PluginManager Plugins;

    DogwaterServer();

    unsigned int GetNextClientId();

    void BeginListening(unsigned short port);
    void HandleMessage(sockaddr_in* socket, char* buffer);

    bool Send(Connection* connection, unsigned char* buffer, unsigned int bufferSize);
    bool Send(Connection* connection, MessageWriter& writer);

    bool Acknowledge(Connection* connection, uint16_t nonce);
    bool Disconnect(Connection* connection, DisconnectReason reason = DisconnectReason::None, const char* customMessage = nullptr);
};

#endif //DOGWATER_DOGWATERSERVER_H
