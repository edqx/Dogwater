#include "enums.h"

#ifndef DOGWATER_DOGWATERPLUGIN_H
#define DOGWATER_DOGWATERPLUGIN_H

class DogwaterPlugin {
public:
    const char* Name;
    const char* Version;
    ModPluginSide PluginSide;

    DogwaterPlugin(const char* name, const char* version, ModPluginSide pluginSide);
};

#endif //DOGWATER_DOGWATERPLUGIN_H
