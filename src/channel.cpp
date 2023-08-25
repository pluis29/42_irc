#include "channel.hpp"

#include "user.hpp"
#include "utils.hpp"

Channel::Channel(std::string name, std::string password)
    : name(name),
      password(password),
      topic_restrict(false),
      invite_only(false),
      have_user_limit(false),
      user_limit(0) {}

Channel::~Channel(void) {}

std::string Channel::get_channel_name(void) const { return name; }

std::vector<User*> Channel::get_user_list(void) const { return _users; }

User* Channel::get_user_in_channel(std::string nick) {
    std::vector<User*>::iterator it = _users.begin();

    if (nick[0] == ':') {
        nick.erase(0, 1);
    }
    for (; it != _users.end(); it++) {
        if ((*it)->get_nick() == nick) {
            return (*it);
        }
    }
    return (NULL);
}

std::string Channel::get_password(void) { return password; }

int Channel::get_channel_size() const { return _users.size(); }

std::string Channel::get_topic(void) const { return _topic; }

void Channel::set_topic(std::string topic) { _topic = topic; }

bool Channel::is_invite_only(void) { return invite_only; }

bool Channel::is_topic_restricted(void) { return topic_restrict; }

bool Channel::is_user_limit(void) { return have_user_limit; }

bool Channel::find_user_in_channel(int user_fd) {
    for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++) {
        if ((*it)->get_user_fd() == user_fd) {
            return true;
        }
    }
    return false;
}

bool Channel::find_channel_oper(void) {
    std::vector<User*>::iterator it = _users.begin();

    for (; it != _users.end(); ++it) {
        if ((*it)->user_channel_info.find(get_channel_name()) != (*it)->user_channel_info.end() &&
            (*it)->user_channel_info[get_channel_name()] == true) {
            return true;
        }
    }
    return false;
}

User* Channel::find_next_channel_oper(int user_fd) {
    for (std::vector<User*>::iterator it = this->_users.begin(); it != this->_users.end(); it++) {
        if ((*it)->get_user_fd() != user_fd) {
            if (!(*it)->user_channel_info[name]) {
                return (*it);
            }
        }
    }
    return NULL;
}

void Channel::message_to_channel(std::string msg, int exclude_fd) {
    std::vector<User*>::iterator it = _users.begin();

    if (msg.find("\r\n") == std::string::npos) {
        msg += "\r\n";
    }
    for (; it != _users.end(); it++) {
        if ((*it)->get_user_fd() != exclude_fd) {
            if (send((*it)->get_user_fd(), msg.c_str(), msg.size(), 0) < 0) {
                Utils::error_message("message_to_channel: send:", strerror(errno));
            }
        }
    }
}

void Channel::message_to_channel(std::string msg) {
    std::vector<User*>::iterator it = _users.begin();

    if (msg.find("\r\n") == std::string::npos) {
        msg += "\r\n";
    }
    for (; it != _users.end(); it++) {
        if (send((*it)->get_user_fd(), msg.c_str(), msg.size(), 0) < 0) {
            Utils::error_message("message_to_channel: send:", strerror(errno));
        }
    }
}

void Channel::add_user(User* user) {
    if (get_user_in_channel(user->get_nick()) == NULL) {
        _users.push_back(user);
    }
}

void Channel::remove_user(User* user) {
    std::vector<User*>::iterator it = this->_users.begin();
    for (; it != this->_users.end(); it++) {
        if (*it == user) {
            this->_users.erase(it);
            return;
        }
    }
}

void Channel::clear_topic(void) { _topic.clear(); }
