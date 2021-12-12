#include "DogwaterServer.h"

DogwaterServer::DogwaterServer() {
    ClientId = 0;
}

unsigned int DogwaterServer::GetNextClientId() {
    ClientId++;
    return ClientId;
}

void DogwaterServer::GetAddressString(const char* out, sockaddr_in *socket) {
    sprintf((char*)out, "%s:%u", inet_ntoa(socket->sin_addr), socket->sin_port);
}

Connection* DogwaterServer::GetConnectionByAddress(sockaddr_in* socket) {
    char addressString[23] = {};
    GetAddressString(addressString, socket);
    return GetConnectionByAddress(addressString);
}

Connection* DogwaterServer::GetConnectionByAddress(const char* address) {
    auto found = ConnectionMap.find(address);

    if (found == ConnectionMap.end()) {
        return nullptr;
    }

    return found->second;
}

void DogwaterServer::RemoveConnection(Connection *connection) {
    char addressString[22] = {};
    GetAddressString(addressString, connection->Socket);

    ConnectionMap.erase(addressString);
    free(connection);
}

void DogwaterServer::BeginListening(unsigned short port) {
    sockaddr_in server, client;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        exit(0);
    }

    if ((ServerSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create Socket: %d\n", WSAGetLastError());
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(ServerSocket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    IsListening = true;

    std::thread([this]()
    {
        while (IsListening) {
            auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(2500);

            auto it = ConnectionMap.begin();
            while (it != ConnectionMap.end())
            {
                MessageWriter writer(3);
                writer.WriteUInt8((uint8_t)SendOption::Ping);
                writer.WriteUInt16(it->second->GetNextNonce(), true);
                it++;
            }


            std::this_thread::sleep_until(x);
        }
    }).detach();

    while (true)
    {
        char buffer[BUFLEN] = {};
        int messageLength;
        int socketLen = sizeof(sockaddr_in);

        if ((messageLength = recvfrom(ServerSocket, buffer, BUFLEN, 0, (sockaddr*)&client, &socketLen)) == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if (err == 10054) {
                continue;
            }

            printf("recvfrom() failed with error code: %d", err);
            exit(0);
        }

        HandleMessage(&client, buffer);
    }

    closesocket(ServerSocket);
    WSACleanup();
}

void DogwaterServer::HandleMessage(sockaddr_in *socket, char* buffer) {
    char addressString[23] = {};
    GetAddressString(addressString, socket);
    Connection* cachedConnection = GetConnectionByAddress(addressString);

    if (cachedConnection != nullptr) {
        MessageReader reader((uint8_t *) buffer, BUFLEN);
        uint8_t packetTag = reader.ReadUInt8();

        if (packetTag == (uint8_t) SendOption::Reliable) {
            uint16_t nonce = reader.ReadUInt16(true);
            Acknowledge(cachedConnection, nonce);
        } else if (packetTag == (uint8_t)SendOption::Disconnect) {
            Disconnect(cachedConnection);
        } else if (packetTag == (uint8_t)SendOption::Ping) {
            uint16_t nonce = reader.ReadUInt16(true);
            Acknowledge(cachedConnection, nonce);
        }
    } else if (buffer[0] == (uint8_t)SendOption::Hello) {
        MessageReader reader((uint8_t*)buffer, BUFLEN);
        reader.Jump(1); // already checked if hello
        uint16_t nonce = reader.ReadUInt16(true);
        reader.Jump(1); // skip hazel version
        uint32_t clientVersion = reader.ReadUInt32();

        const char* username = reader.ReadString();
        uint32_t authToken = reader.ReadUInt32();
        uint32_t language = reader.ReadUInt32();
        uint8_t chatMode = reader.ReadUInt8();

        MessageReader platformReader = reader.ReadMessage();
        const char* platformName = platformReader.ReadString();

        if (platformReader.MessageTag == (uint8_t)Platform::Xbox || platformReader.MessageTag == (uint8_t)Platform::Playstation) {
            platformReader.Jump(8); // skip console platform id
        }

        reader.ReadInt32();

        unsigned int clientId = GetNextClientId();

        PlatformSpecificData* platform = new PlatformSpecificData{
                (Platform)platformReader.MessageTag,
                platformName
        };

        auto* newConnection = new Connection(socket, username, clientId, platform);
        ConnectionMap.insert(std::make_pair(addressString, newConnection));

        if (clientVersion != 0x3034262) {
            Disconnect(newConnection, DisconnectReason::IncorrectVersion);
            return;
        }

        if (reader.BytesRemaining()) { // probably a reactor modded hello
            uint8_t protocolVersion = reader.ReadUInt8();
            uint32_t modCount = reader.ReadPackedUInt32();

            MessageWriter handshakeWriter(7 + strlen("Dogwater") + strlen("1.0.0") + 1);
            handshakeWriter.WriteUInt8((uint8_t)SendOption::Reliable);
            handshakeWriter.WriteUInt16(newConnection->GetNextNonce(), true);
            handshakeWriter.BeginMessage(0xff);
            handshakeWriter.WriteUInt8((uint8_t)ReactorMessageTag::Handshake);
            handshakeWriter.WriteString("Dogwater");
            handshakeWriter.WriteString("1.0.0");
            handshakeWriter.WritePackedInt32(Plugins.LoadedPlugins.size());
            handshakeWriter.EndMessage();
            Send(newConnection, handshakeWriter);

            MessageWriter pluginsWriter(512);
            pluginsWriter.WriteUInt8((uint8_t)SendOption::Reliable);
            pluginsWriter.WriteUInt16(newConnection->GetNextNonce(), true);
            unsigned int i = 0;
            for (const auto& plugin : Plugins.LoadedPlugins) {
                pluginsWriter.BeginMessage(0xff);
                pluginsWriter.WriteUInt8((uint8_t)ReactorMessageTag::PluginDeclaration);
                pluginsWriter.WritePackedUInt32(i);
                pluginsWriter.WriteString(plugin->Name);
                pluginsWriter.WriteString(plugin->Version);
                pluginsWriter.WriteUInt8((uint8_t)plugin->PluginSide);
                pluginsWriter.EndMessage();
                i++;
            }
            pluginsWriter.ShrinkToSize();
            Send(newConnection, pluginsWriter);
        }

        printf("Connection created: %s (id=%i,ip=%s)\n", username, clientId, addressString);

        Acknowledge(newConnection, nonce);
    }
}

bool DogwaterServer::Send(Connection* connection, unsigned char* buffer, unsigned int bufferSize) {
    if (sendto(ServerSocket, (const char*)buffer, bufferSize, 0, (struct sockaddr*)connection->Socket, sizeof(*connection->Socket)) == SOCKET_ERROR)
    {
        return false;
    }

    return true;
}

bool DogwaterServer::Send(Connection* connection, MessageWriter &writer) {
    return Send(connection, writer.Buffer, writer.BufferSize);
}

bool DogwaterServer::Acknowledge(Connection* connection, uint16_t nonce) {
    MessageWriter ackWriter(4);
    ackWriter.WriteUInt8((uint8_t)SendOption::Acknowledge);
    ackWriter.WriteUInt16(nonce, true);
    ackWriter.WriteUInt8(0xff);

    return Send(connection, ackWriter);
}

bool DogwaterServer::Disconnect(Connection *connection, DisconnectReason reason, const char* customMessage) {
    char addressString[22] = { };
    GetAddressString(addressString, connection->Socket);
    printf("Connection closed: %s (id=%i,ip=%s)\n", connection->Username, connection->ClientId, addressString);
    if (reason == DisconnectReason::None) {
        unsigned char buffer[1] = { (uint8_t)SendOption::Disconnect };
        return Send(connection, buffer, 1);
    }
    MessageWriter dcWriter(6 + (customMessage != nullptr ? strlen(customMessage) + 1 : 0));
    dcWriter.WriteUInt8((uint8_t)SendOption::Disconnect);
    dcWriter.WriteBool(true);
    dcWriter.BeginMessage(0);
    dcWriter.WriteUInt8((uint8_t)reason);
    if (reason == DisconnectReason::Custom && customMessage != nullptr) {
        dcWriter.WriteString(customMessage);
    }
    dcWriter.EndMessage();
    Send(connection, dcWriter);
    connection->SentDisconnected = true;
    RemoveConnection(connection);
}