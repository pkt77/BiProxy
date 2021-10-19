cmake_minimum_required(VERSION 3.17)
project(BiProxy)

set(CMAKE_CXX_STANDARD 17)

#Zlib Compression
add_subdirectory(lib/zlib "${CMAKE_BINARY_DIR}/zlib-build")
include_directories(lib/zlib "${CMAKE_BINARY_DIR}/zlib-build")

#YAML
add_subdirectory(lib/yaml-cpp)
include_directories(lib/yaml-cpp/include)

#REST API
#vcpkg install cpprestsdk cpprestsdk:x64-windows
#sudo apt-get install libcpprest-dev
#cmake params: -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake
find_package(cpprestsdk CONFIG REQUIRED)

#Encryption
find_package(OpenSSL REQUIRED)
#include_directories(lib/openssl/include)

include_directories(include)

if (WIN32)
    set(SOCKET_HANDLER src/net/WinSock.cpp src/net/WinSockTCP.cpp src/net/WinSockUDP.cpp)
    link_libraries(ws2_32 wsock32)
endif (WIN32)

if (UNIX)
    set(SOCKET_HANDLER src/net/UnixSock.cpp src/net/UnixSockTCP.cpp src/net/UnixSockUDP.cpp)
endif (UNIX)

file(GLOB SOURCE_FILES src/* src/net/JavaPacketHandler.cpp src/net/BedrockClientHandler.cpp src/net/RakNetPacketHandler.cpp src/data/* src/packets/* src/utils/*)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SOCKET_HANDLER})
target_link_libraries(${PROJECT_NAME} PRIVATE cpprestsdk::cpprest zlibstatic yaml-cpp OpenSSL::SSL OpenSSL::Crypto)