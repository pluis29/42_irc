#include "utils.hpp"
#include <algorithm>

void Utils::error_message(std::string function, std::string error) {
    throw std::runtime_error("ERROR " + function + ": " + error);
}

std::vector<std::string> Utils::split(std::string str, char c) {
    std::string buff = "";
    std::vector<std::string> split;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != c)
            buff += str[i];
        else if (str[i] == c && buff != "") {
            split.push_back(buff);
            buff = "";
        }
    }
    if (buff != "") split.push_back(buff);
    return split;
}

std::string Utils::toUpperCase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}
