#include "channel.hpp"

#include <unistd.h>

#include <map>
#include <vector>

#include "user.hpp"
#include "utils.hpp"

Channel::Channel(std::string name, std::string password) : invite_only(false), _name(name), password(password) {
    return;
}

Channel::~Channel(void) { return; }

std::string Channel::get_channel_name(void) const { return _name; }

std::vector<User*> Channel::get_user_list(void) const { return _users; }

User* Channel::get_user_in_channel(std::string nick) {
    std::vector<User*>::iterator it = this->_users.begin();

    if (nick[0] == ':') nick.erase(0, 1);
    for (; it != this->_users.end(); it++) {
        if ((*it)->get_nick() == nick) return (*it);
    }
    return (NULL);
}

std::string Channel::get_password(void) { return this->password; }

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
        if ((*it)->user_channel_info.find(get_channel_name()) != (*it)->user_channel_info.end() &&
            (*it)->user_channel_info[get_channel_name()] == true) {
            return true;  // O usuário é operador no canal "nome_do_canal"
        }
    }
    return false;  // Nenhum usuário é operador no canal "nome_do_canal"
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

/* User* Channel::find_next_channel_oper(int user_fd) { */
/*     for (std::vector<User*>::iterator it = this->_users.begin(); it != this->_users.end(); it++) { */
/*         std::cout << "k1\n"; */
/*         if ((*it)->get_user_fd() != user_fd) { */
/*             std::cout << "k2\n"; */
/*             std::map<std::string, bool>::iterator map_it = (*it)->user_channel_info.find(_name); */
/*             if (map_it != (*it)->user_channel_info.end()) { */
/*                 std::cout << "k3: Found map entry\n"; */
/*                 if (map_it->second) { */
/*                     std::cout << "k4: Entry is true\n"; */
/*                 } else { */
/*                     std::cout << "k5: Entry is false\n"; */
/*                     return (*it); */
/*                 } */
/*             } else { */
/*                 std::cout << "k6: Entry not found\n"; */
/*             } */
/*         } */
/*     } */
/*     return NULL; */
/* } */

User* Channel::find_next_channel_oper(int user_fd) {
    for (std::vector<User*>::iterator it = this->_users.begin(); it != this->_users.end(); it++) {
        if ((*it)->get_user_fd() != user_fd) {
            if (!(*it)->user_channel_info[_name]) {
                return (*it);
            }
        }
    }
    return NULL;
}
int Channel::get_channel_size() const { return _users.size(); }

void Channel::clear_topic(void) { _topic.clear(); }

void Channel::set_topic(std::string topic) { _topic = topic; }

std::string Channel::get_topic(void) const { return _topic; }

bool Channel::is_invite_only(void) { return invite_only; }
