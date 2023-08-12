#include "utils.hpp"

void Utils::errorMessage(std::string function, std::string error) {
    throw std::runtime_error(function + "  " + error);
}
