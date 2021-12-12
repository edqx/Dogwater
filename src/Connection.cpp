#include "Connection.h"

Connection::Connection(sockaddr_in *socket, const char* username, unsigned int clientId, PlatformSpecificData* platform) {
    NextNonce = 0;
    SentDisconnected = false;
    Socket = socket;
    Username = username;
    ClientId = clientId;
    Platform = platform;
}

Connection::~Connection() {
    free((void*)Username);
    free((void*)Platform->PlatformName);
    free((void*)Platform);
}

unsigned int Connection::GetNextNonce() {
    NextNonce++;
    return NextNonce;
}