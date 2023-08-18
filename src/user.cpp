#include "user.hpp"

User::User(int user_fd) : _user_fd(user_fd), _nick(""), _username(""), _auth(false), _oper(false) { return; }

User::~User(void) { close(this->_user_fd); }

int User::get_fd(void) { return this->_user_fd; }

std::string User::get_username(void) { return this->_username; }

std::string User::get_nick(void) { return this->_nick; }

void User::set_auth(void) { this->_auth = true; }

int User::check_auth(void) { return this->_auth; }

void User::set_nick(std::string nick) { this->_nick = nick; }

void User::set_username(std::string username) { this->_username = username; }

void User::set_hostname(std::string hostname) { this->_hostname = hostname; }

void User::set_realname(std::string realname) { this->_realname = realname; }

void User::set_servername(std::string servername) { this->_servername = servername; }

std::string User::get_hostname(void) { return this->_hostname; }

void User::set_operator(void) { this->_oper = !this->_oper; }

bool User::is_oper(void) { return (this->_oper); }
