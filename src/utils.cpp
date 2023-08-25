#include "utils.hpp"

void Utils::error_message(std::string function, std::string error) {
    throw std::runtime_error("ERROR " + function + ": " + error);
}

std::vector<std::string> Utils::split(std::string str, char c) {
    std::string buff = "";
    std::vector<std::string> split;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != c) {
            buff += str[i];
        } else if (str[i] == c && buff != "") {
            split.push_back(buff);
            buff = "";
        }
    }
    if (buff != "") {
        split.push_back(buff);
    }
    return split;
}

std::string Utils::toUpperCase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

bool Utils::check_invalid_char(std::string str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalnum(str[i]) && !isalpha(str[i])) {
            return true;
        }
    }
    return false;
}

std::string Utils::joinToString(std::vector<std::string>::iterator first, std::vector<std::string>::iterator last) {
    std::string message;

    for (; first != last; first++) {
        message += *first + " ";
    }
    return (message);
}
