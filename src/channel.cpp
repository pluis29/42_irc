#include "channel.hpp"

#include <unistd.h>

#include <map>
#include <vector>

#include "user.hpp"
#include "utils.hpp"

Channel::Channel(std::string name, std::string password) : _name(name), _password(password) { return; }

Channel::~Channel(void) { return; }

std::string Channel::get_channel_name(void) const { return this->_name; }

User* Channel::get_user_in_channel(std::string nick) {
    std::vector<User*>::iterator it = this->_users.begin();

    if (nick[0] == ':') nick.erase(0, 1);
    for (; it != this->_users.end(); it++) {
        if ((*it)->get_nick() == nick) return (*it);
    }
    return (NULL);
}

std::string Channel::get_password(void) { return this->_password; }

void Channel::message_to_channel(std::string msg, int exclude_fd) {
    std::vector<User*>::iterator it = this->_users.begin();

    if (msg.find("\r\n") == std::string::npos) msg += "\r\n";
    for (; it != this->_users.end(); it++)
        if ((*it)->get_user_fd() != exclude_fd)
            if (send((*it)->get_user_fd(), msg.c_str(), strlen(msg.c_str()), 0) < 0)
                Utils::error_message("message_to_channel: send:", strerror(errno));
    return;
}

void Channel::message_to_channel(std::string msg) {
    std::vector<User*>::iterator it = this->_users.begin();

    if (msg.find("\r\n") == std::string::npos) msg += "\r\n";
    for (; it != this->_users.end(); it++)
        if (send((*it)->get_user_fd(), msg.c_str(), strlen(msg.c_str()), 0) < 0)
            Utils::error_message("message_to_channel: send:", strerror(errno));
    return;
}



void Channel::add_user(User* user) {
    if (get_user_in_channel(user->get_nick()) == NULL) this->_users.push_back(user);
    return;
}









bool Channel::find_channel_oper(void) {
    std::vector<User*>::iterator it = _users.begin();

    for (; it != _users.end(); ++it) {
        if ((*it)->user_channels.find(get_channel_name()) != (*it)->user_channels.end() &&
            (*it)->user_channels[get_channel_name()] == true) {
            return true;  // O usuário é operador no canal "nome_do_canal"
        }
    }
    return false;  // Nenhum usuário é operador no canal "nome_do_canal"
}
