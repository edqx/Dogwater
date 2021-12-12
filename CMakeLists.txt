cmake_minimum_required(VERSION 3.21)
project(Dogwater)

set(CMAKE_CXX_STANDARD 20)

add_executable(Dogwater src/main.cpp src/util/MessageReader.cpp src/util/MessageReader.h src/DogwaterServer.cpp src/DogwaterServer.h src/enums.h src/Connection.cpp src/Connection.h src/util/MessageWriter.cpp src/util/MessageWriter.h src/DogwaterPlugin.cpp src/DogwaterPlugin.h src/PluginManager.cpp src/PluginManager.h src/util/bswap.h)
target_link_libraries(Dogwater ws2_32)