#include <utils/Utils.h>

#include <random>
#include <climits>
#include <chrono>

int randomInt() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(INT_MIN, INT_MAX);

    return dist(mt);
}

long long randomLong() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<long long> dist(LLONG_MIN, LLONG_MAX);

    return dist(mt);
}

long long currentTimeMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

char varIntLength(int value) {
    if ((value & 0xFFFFFF80) == 0) {
        return 1;
    }

    if ((value & 0xFFFFC000) == 0) {
        return 2;
    }

    if ((value & 0xFFE00000) == 0) {
        return 3;
    }

    if ((value & 0xF0000000) == 0) {
        return 4;
    }

    return 5;
}

std::pair<std::string, unsigned short> parseHost(std::string host) {
    for (int i = 0; i < host.length(); i++) {
        if (host[i] == ':') {
            return std::pair<std::string, unsigned short>(host.substr(0, i), std::stoi(host.substr(i + 1, host.length())));
        }
    }

    throw "Failed to parse host";
}