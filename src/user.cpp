#include "user.hpp"

#include <string>

#include "utils.hpp"

User::User(int user_fd)
    : _user_fd(user_fd), _nick(""), _username(""), _general_auth(false), _password_auth(false), _oper(false) {
    return;
}

User::~User(void) { close(_user_fd); }

bool User::is_general_auth(void) const { return _general_auth; }

bool User::is_password_auth(void) const { return _password_auth; }




std::string User::get_hostname(void) const { return _hostname; }

std::string User::get_nick(void) const { return _nick; }

std::string User::get_servername(void) const { return _servername; }

std::string User::get_username(void) const { return _username; }

int User::get_user_fd(void) const { return _user_fd; }





void User::set_password_auth(void) { _password_auth = true; }

void User::set_nick(std::string nick) { _nick = nick; }

void User::set_general_auth(void) { _general_auth = true; }

void User::set_hostname(std::string user_ip) { _hostname = user_ip; }

void User::set_username(std::string username) { this->_username = username; }

void User::set_realname(std::string realname) { this->_realname = realname; }

void User::set_servername(std::string servername) { this->_servername = servername; }
