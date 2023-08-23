#include "user.hpp"

#include <map>

#include "channel.hpp"
#include "utils.hpp"

User::User(int user_fd)
    : _user_fd(user_fd), _nick(""), _username(""), _general_auth(false), _password_auth(false), _server_oper(false) {
    return;
}

User::~User(void) { close(_user_fd); }

bool User::is_general_auth(void) const { return _general_auth; }

bool User::is_password_auth(void) const { return _password_auth; }

bool User::is_server_oper(void) const { return _server_oper; }

std::string User::get_hostname(void) const { return _hostname; }

std::string User::get_nick(void) const { return _nick; }

std::string User::get_servername(void) const { return _servername; }

std::string User::get_username(void) const { return _username; }

int User::get_user_fd(void) const { return _user_fd; }

void User::add_channel(Channel* channel) {
    std::map<std::string, bool>::iterator it = user_channel_info.find(channel->get_channel_name());
    // Verifica se o canal já existe nos canais do usuário
    if (it != user_channel_info.end()) {
        return;  // Canal já existe, não faz nada
    }

    user_channel_info.insert(std::make_pair(channel->get_channel_name(), false));
    if (!channel->find_channel_oper()) {
        user_channel_info[channel->get_channel_name()] = true;
        std::string response = ":" + _nick + "!" + _username + "@" + _hostname +
                               " MODE :" + channel->get_channel_name() + " +o " + _nick + "\r\n";
        if (send(_user_fd, response.c_str(), strlen(response.c_str()), 0) < 0)
            Utils::error_message("send:", strerror(errno));
    }

    channel->add_user(this);
    return;
}

void User::set_server_oper(void) { _server_oper = !_server_oper; }

void User::set_password_auth(void) { _password_auth = true; }

void User::set_nick(std::string nick) { _nick = nick; }

void User::set_general_auth(void) { _general_auth = true; }

void User::set_hostname(std::string user_ip) { _hostname = user_ip; }

void User::set_username(std::string username) { this->_username = username; }

void User::set_realname(std::string realname) { this->_realname = realname; }

void User::set_servername(std::string servername) { this->_servername = servername; }
