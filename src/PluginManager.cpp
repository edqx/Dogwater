#include "PluginManager.h"

bool PluginManager::LoadPlugin(DogwaterPlugin* plugin) {
    LoadedPlugins.push_back(plugin);
}

bool PluginManager::UnloadPlugin(DogwaterPlugin* plugin) {
    auto it = std::find(LoadedPlugins.begin(), LoadedPlugins.end(), plugin);

    if (it == LoadedPlugins.end())
        return false;

    LoadedPlugins.erase(it);
    return true;
}