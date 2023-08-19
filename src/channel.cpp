#include "channel.hpp"

#include "user.hpp"
#include "utils.hpp"

Channel::Channel(std::string name, std::string password) : _name(name), _password(password) { return; }

Channel::~Channel(void) { return; }

std::string Channel::get_name(void) { return this->_name; }

std::vector<User*> Channel::get_users(void) { return this->_users; }

User* Channel::get_user_in_channel(std::string nick) {
    std::vector<User*>::iterator it = this->_users.begin();

    if (nick[0] == ':') nick.erase(0, 1);
    for (; it != this->_users.end(); it++) {
        if ((*it)->get_nick() == nick) return (*it);
    }
    return (NULL);
}

std::string Channel::get_password(void) { return this->_password; }

void Channel::add_user(User* user) {
    if (get_user_in_channel(user->get_nick()) == NULL) this->_users.push_back(user);
    return;
}

void Channel::message_to_channel(std::string msg) {
    std::vector<User*>::iterator it = this->_users.begin();

    if (msg.find("\r\n") == std::string::npos) msg += "\r\n";
    for (; it != this->_users.end(); it++)
        if (send((*it)->get_fd(), msg.c_str(), strlen(msg.c_str()), 0) < 0)
            Utils::error_message("messageFromChannel: send:", strerror(errno));
    return;
}

void Channel::remove_user(User* user) {
    std::vector<User*>::iterator it = this->_users.begin();
    for (; it != this->_users.end(); it++) {
        if (*it == user) {
            this->_users.erase(it);
            return;
        }
    }
    return;
}
