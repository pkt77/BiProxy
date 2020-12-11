#include <string>
#include <iostream>

#include "Proxy.h"

bool startsWith(const char* pre, const char* str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

int main(int argc, char** argv) {
/*    char* host;
    unsigned short port = 25565;

    for (int i = 0; i < argc; i++) {
        if (startsWith("host=", argv[i])) {
            host = argv[i] + 5;
        } else if (startsWith("port=", argv[i])) {
            port = std::stoi(std::string(argv[i] + 5));
        }
    }*/

    Proxy proxy;

    while (proxy.isRunning()) {
        char cmdBuffer[1024];

        std::cin >> cmdBuffer;

        if (strncmp(cmdBuffer, "end", 3) == 0) {
            std::cout << "Shutting down..." << std::endl;
            proxy.shutdown();
        }
    }
}