#include <utils/Utils.h>

#include <random>
#include <climits>
#include <chrono>

long long randomLong() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<long long> dist(LLONG_MIN, LLONG_MAX);

    return dist(mt);
}

long long currentTimeMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}