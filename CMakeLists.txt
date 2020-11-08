cmake_minimum_required(VERSION 3.17)
project(BiProxy)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

if (WIN32)
    set(SOCKET_HANDLER src/net/WinSock.cpp src/net/WinSockTCP.cpp src/net/WinSockUDP.cpp)
    link_libraries(ws2_32 wsock32)
endif (WIN32)

if (UNIX)
    #set(SOCKET_HANDLER src/Net/UniSock.cpp)
endif (UNIX)

file(GLOB SOURCE_FILES src/* src/net/* src/packets/* src/utils/*)

add_executable(BiProxy ${SOURCE_FILES} ${SOCKET_HANDLER})