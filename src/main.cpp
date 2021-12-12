#include <iostream>

#include "util/MessageReader.h"
#include "util/MessageWriter.h"

#include "./DogwaterServer.h"

#pragma warning(disable:4996)

#define BUFLEN 1024

int main() {
    DogwaterServer server;

    DogwaterPlugin* plugin = new DogwaterPlugin("dogwater-custom-game-code", "1.0.0.", ModPluginSide::Both);
    server.Plugins.LoadPlugin(plugin);

    server.BeginListening(22023);
    printf("Listening on *:22023\n");
}
