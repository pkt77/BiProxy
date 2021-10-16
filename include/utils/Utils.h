#pragma once

#include <string>

extern int randomInt();

extern long long randomLong();

extern long long currentTimeMillis();

extern char varIntLength(int value);

extern std::pair<std::string, unsigned short> parseHost(std::string host);