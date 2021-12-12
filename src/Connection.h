#include <winsock2.h>

#include "enums.h"

#ifndef DOGWATER_CONNECTION_H
#define DOGWATER_CONNECTION_H

struct PlatformSpecificData {
    Platform PlatformTag;
    const char* PlatformName;
};

class Connection {
    unsigned int NextNonce;
public:
    const char* Username;
    unsigned int ClientId;
    PlatformSpecificData* Platform;

    bool SentDisconnected;
    sockaddr_in* Socket;

    unsigned int GetNextNonce();

    Connection(sockaddr_in* socket, const char* username, unsigned int clientId, PlatformSpecificData* platform);
    ~Connection();
};

#endif //DOGWATER_CONNECTION_H
