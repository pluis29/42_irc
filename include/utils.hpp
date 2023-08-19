#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

class User;
namespace Utils {
void error_message(std::string function, std::string error);
std::vector<std::string> split(std::string str, char c);
std::string toUpperCase(std::string str);
bool check_invalid_char(std::string str);
std::string joinToString(std::vector<std::string>::iterator first, std::vector<std::string>::iterator last);


std::string joinToString(std::vector<User *> users);
}  // namespace Utils

#endif
