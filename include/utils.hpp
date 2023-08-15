#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <stdexcept>
#include <vector>

namespace Utils {
void error_message(std::string function, std::string error);
std::vector<std::string> split(std::string str, char c);
std::string toUpperCase(std::string str);
}  // namespace Utils

#endif
