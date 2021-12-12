#include <vector>

#include "DogwaterPlugin.h"

#ifndef DOGWATER_PLUGINMANAGER_H
#define DOGWATER_PLUGINMANAGER_H

class PluginManager {
public:
    std::vector<DogwaterPlugin*> LoadedPlugins;

    bool LoadPlugin(DogwaterPlugin* plugin);
    bool UnloadPlugin(DogwaterPlugin* plugin);
};

#endif //DOGWATER_PLUGINMANAGER_H
