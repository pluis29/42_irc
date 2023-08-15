#include "user.hpp"

User::User(int user_fd)
    : _user_fd(user_fd), _nick(""), _username(""), _auth(false), _oper(false) {
    return;
}

User::~User(void) { close(this->_user_fd); }

int User::get_fd(void) { return this->_user_fd; }

std::string User::get_username(void) { return this->_username; }

std::string User::get_nick(void) { return this->_nick; }

void User::set_auth(void) { this->_auth = true; }

int User::check_auth(void) { return this->_auth; }
