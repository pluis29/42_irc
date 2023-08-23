#include "command.hpp"

#include "channel.hpp"
#include "server.hpp"
#include "user.hpp"
#include "utils.hpp"

Command::Command(std::string buffer, User& user, Server& server) : _user(user), _server(server) {
    _parse_buffer(buffer);
    _handle_command();
}

Command::~Command(void) { return; }

void Command::_parse_buffer(std::string buffer) {
    if (buffer[0] == '/') buffer.erase(0, 1);
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'), buffer.end());

    _args = Utils::split(buffer, ' ');
    if (_args.size() == 0) return;

    _command = Utils::toUpperCase(*_args.begin());
    _args.erase(_args.begin());
    if (_args[0][0] == ':') _args[0].erase(0, 1);
}

void Command::_handle_command(void) {
    std::map<std::string, command_handler> command_map;

    command_map["PASS"] = &Command::_command_pass;  // --
    command_map["NICK"] = &Command::_command_nick;  // --
    command_map["USER"] = &Command::_command_user;  // --
    command_map["QUIT"] = &Command::_command_quit;
    if (_user.is_general_auth()) {
        command_map["OPER"] = &Command::_command_oper;  //--
        command_map["JOIN"] = &Command::_command_join;
        command_map["WHO"] = &Command::_command_who;          //--
        command_map["PRIVMSG"] = &Command::_command_privmsg;  //--
        command_map["PART"] = &Command::_command_part;
    }

    std::map<std::string, command_handler>::iterator it = command_map.find(this->_command);
    if (it != command_map.end()) {
        (this->*(it->second))();
        return;
    } else
        return _invalid_command();
}

void Command::_invalid_command(void) {
    if (!_user.is_password_auth()) {
        _message_to_user("Authenticate is required: usage: /PASS <password>", "464");
        return;
    }
    if (_user.get_nick().empty()) {
        _message_to_user("A nick must be provide: usage: /NICK <nick>", "431");
        return;
    }
    if (_user.get_username().empty()) {
        _message_to_user("A user must be provide: usage: /USER <username> <hostname> "
                         "<servername> <realname>",
                         "431");
        return;
    }
    _message_to_user(_command + " :Unknown command", "421");
}

void Command::_message_to_user(std::string msg, std::string code, int fd, std::string opt) {
    std::string response =
        ":" + _user.get_servername() + " " + code + " " + (_user.get_nick().empty() ? "*" : _user.get_nick()) + " ";
    if (!opt.empty()) response += opt + " ";
    response += msg + "\r\n";
    if (fd == 0) fd = _user.get_user_fd();
    if (send(fd, response.c_str(), response.size(), 0) < 0) Utils::error_message("send:", strerror(errno));
}

//-------------------commands---------------------------------------------

void Command::_command_pass(void) {
    if (_user.is_password_auth()) return (_message_to_user(":Unauthorized command (already registered)", "462"));
    if (_args.empty() || _args.size() != 1) return (_message_to_user(":Not enough parameters", "461"));
    if (_args[0] == _server.get_password()) {
        _user.set_password_auth();
        return (_message_to_user(":Password Correct", "001"));
    } else
        return (_message_to_user(":Password incorrect", "464"));
}

void Command::_command_nick(void) {
    if (!_user.is_password_auth()) return _invalid_command();
    if (_args.size() != 1) return (_message_to_user(":No nickname given", "431"));
    if (_args[0].empty() || Utils::check_invalid_char(_args[0]))
        return (_message_to_user(":Erroneous nickname", "432"));

    std::vector<User*> users;
    users = _server.get_users_in_server();
    for (std::vector<User*>::iterator it = users.begin(); it != users.end(); it++) {
        if (_args[0] == (*it)->get_nick()) return (_message_to_user(":Nickname is already in use", "433"));
    }

    std::string response;
    std::string prevNick = _user.get_nick();
    _user.set_nick(_args[0]);
    if (prevNick.empty()) {
        response = ":" + _user.get_nick() + "!" + _user.get_username() + "@" + _user.get_hostname() +
                   " NICK :" + _user.get_nick() + "\r\n";
    } else {
        response = ":" + prevNick + "!" + _user.get_username() + "@" + _user.get_hostname() +
                   " NICK :" + _user.get_nick() + "\r\n";
    }
    if (send(_user.get_user_fd(), response.c_str(), response.size(), 0) < 0)
        Utils::error_message("send:", strerror(errno));
    _server.message_all_users(response, _user.get_user_fd());
}

void Command::_command_user(void) {
    if (!_user.is_password_auth() || _user.get_nick().empty()) return _invalid_command();

    if (this->_args.size() != 4) return _message_to_user(":Not enough parameters", "461");

    if (this->_user.get_username().empty() == false)
        return _message_to_user(":Unauthorized command (already registered)", "462");

    this->_user.set_username(this->_args[0]);
    this->_user.set_realname(this->_args[3]);

    _message_to_user("Welcome to the ft_irc " + this->_user.get_nick() + "!" + this->_user.get_username() + "@" +
                         this->_user.get_hostname(),
                     "001");

    _user.set_general_auth();
    if (!_server.find_server_oper()) {
        _user.set_server_oper();
        _message_to_user(":You are now an IRC operator", "381");
    }
}

void Command::_command_quit(void) {
    if (this->_user.is_server_oper()) this->_user.set_server_oper();

    if (_server.get_users_in_server().size() != 1 && !_server.find_server_oper()) {
        User* user;
        user = _server.find_next_server_oper(this->_user.get_user_fd());
        if (user != NULL) {
            user->set_server_oper();
            _message_to_user(":You are now an IRC operator", "381", user->get_user_fd());
        }
    }
    std::vector<std::string> channels_to_flush = set_next_channel_oper(true);

    std::cout << "User left. FD: " << this->_user.get_user_fd() << std::endl;
    _server.delete_user(this->_user.get_user_fd());
    if (!channels_to_flush.empty()) {
        for (std::vector<std::string>::iterator it = channels_to_flush.begin(); it != channels_to_flush.end(); ++it)
            _flush_hex((*it));
    }
}

void Command::_command_oper(void) {
    User* user;

    if (_args.size() != 2) return (_message_to_user(":Not enough parameters", "461"));
    user = _server.get_user_byNick(_args[0]);
    if (user == NULL) return (_message_to_user(":No such nick", "401"));
    if (user->is_server_oper()) return (_message_to_user(":You are already an operator.", "690"));
    if (_args[1] != OPERATOR_PASS) return (_message_to_user(":Password incorrect", "464"));
    user->set_server_oper();
    _message_to_user(":You are now an IRC operator", "381", user->get_user_fd());
}

void Command::_command_join(void) {
    Channel* channel;
    std::string users;
    std::string password;

    if (_args[0][0] != '#') _args[0] = "#" + _args[0];
    if (_args.size() < 1 || _args.size() > 2) return (_message_to_user(":Not enough parameters", "461"));
    if (Utils::check_invalid_char(_args[0].c_str() + 1)) return (_message_to_user(":No such channel", "403"));

    password = _args.size() == 1 ? "" : _args[1];
    channel = _server.get_channel_by_name(_args[0]);
    if (channel == NULL) {
        channel = new Channel(_args[0], password);
        _server.add_channel(channel);  // add the channel in the vector of channels
        /* _give_oper_to_creator(channel->get_name()); */
    }

    if (channel->get_user_in_channel(this->_user.get_nick()) != NULL)
        return (_message_to_user(":is already on channel", "443"));
    if (password == channel->get_password())
        this->_user.add_channel(channel);
    else
        return _message_to_user(":Password incorrect", "464");
    channel->message_to_channel(":" + this->_user.get_nick() + " JOIN " + channel->get_channel_name());
}

void Command::_command_who(void) {
    if (this->_args.size() > 2) return _message_to_user(":Not enough parameters", "461");

    std::string target = _args.size() > 0 ? _args[0] : "";
    if (target.empty()) _list_server_oper();
    if (target[0] == '#') _list_channel_oper(target);
}

void Command::_list_server_oper(void) {
    std::string response;
    std::vector<User*> users = _server.get_users_in_server();
    int i = 0;

    for (std::vector<User*>::iterator it = users.begin(); it != users.end(); it++) {
        std::ostringstream oss;
        oss << i;
        std::string indexString = oss.str();

        response = ":" + this->_user.get_servername() + " 352 " + this->_user.get_nick() + " * " +
                   (*it)->get_username() + " " + (*it)->get_hostname() + " " + (*it)->get_servername();
        response += (*it)->is_server_oper() ? " H :" + indexString + " @" : " :" + indexString + " ";
        response += (*it)->get_nick() + "\r\n";
        if (send(this->_user.get_user_fd(), response.c_str(), response.size(), 0) < 0)
            Utils::error_message("send:", strerror(errno));
        i++;
    }
    response = ":" + _user.get_servername() + " " + "315" + " " + _user.get_nick() + " " + ":End of /WHO list.\r\n";
    if (send(this->_user.get_user_fd(), response.c_str(), response.size(), 0) < 0)
        Utils::error_message("send:", strerror(errno));
}

void Command::_list_channel_oper(std::string channel_name) {
    std::string response;
    Channel* channel = _server.get_channel_by_name(channel_name);

    if (channel == NULL) return _message_to_user(":No such channel", "403");
    _flush_hex(this->_args[0]);

    std::vector<User*> tmp = channel->get_user_list();
    int i = 0;
    for (std::vector<User*>::iterator it = tmp.begin(); it != tmp.end(); it++) {
        std::ostringstream oss;
        oss << i;
        std::string indexString = oss.str();
        response = ":" + this->_user.get_servername() + " 352 " + this->_user.get_nick() + " :" +
                   channel->get_channel_name() + " " + (*it)->get_username() + " " + (*it)->get_hostname() + " " +
                   (*it)->get_servername();
        if ((*it)->user_channel_info.find(channel->get_channel_name()) != (*it)->user_channel_info.end() &&
            (*it)->user_channel_info[channel->get_channel_name()] == true) {
            response += " H :" + indexString + " @";
        } else {
            response += " :" + indexString + " ";
        }
        response += (*it)->get_nick() + "\r\n";

        if (send(this->_user.get_user_fd(), response.c_str(), response.size(), 0) < 0)
            Utils::error_message("send:", strerror(errno));
        i++;
    }
    response = ":" + _user.get_servername() + " " + "315" + " " + _user.get_nick() + " " + ":End of /WHO list.\r\n";
    if (send(this->_user.get_user_fd(), response.c_str(), response.size(), 0) < 0)
        Utils::error_message("send:", strerror(errno));
}

void Command::_flush_hex(std::string channel_target) {
    std::string response;

    Channel* channel = _server.get_channel_by_name(channel_target);

    std::vector<User*> user_list = channel->get_user_list();
    for (std::vector<User*>::iterator listIt = user_list.begin(); listIt != user_list.end(); listIt++) {
        std::vector<User*> tmp = channel->get_user_list();
        response = ":" + (*listIt)->get_servername() + " " + "353" + " " + (*listIt)->get_nick() + " = " +
                   channel->get_channel_name() + " :";
        for (std::vector<User*>::iterator it = tmp.begin(); it != tmp.end(); it++) {
            response += (*it)->user_channel_info.find(channel->get_channel_name()) != (*it)->user_channel_info.end() &&
                                (*it)->user_channel_info[channel->get_channel_name()] == true
                            ? "@"
                            : "";
            response += (*it)->get_nick() + " ";
        }
        response += "\r\n:" + (*listIt)->get_servername() + " " + "366" + " " + (*listIt)->get_nick() + " " +
                    channel->get_channel_name() + " :End of /NAMES list.\r\n";
        if (send((*listIt)->get_user_fd(), response.c_str(), response.size(), 0) < 0)
            Utils::error_message("send:", strerror(errno));
    }
}

void Command::_command_privmsg(void) {
    if (_args.size() == 0) return _message_to_user(":No recipient given", "411");
    if (_args.size() == 1) return _message_to_user(":No text to send", "412");

    std::string target = _args[0];
    std::string message = Utils::joinToString(_args.begin() + 1, _args.end());
    std::string response;

    if (target[0] != '#') {
        User* receive = _server.get_user_byNick(target);
        if (receive == NULL) return _message_to_user(":No such nick/channel", "401");
        if (message[0] == ':') message.erase(0, 1);
        response = ":" + _user.get_nick() + " PRIVMSG " + receive->get_nick() + " :" + message;
        receive->send_message_to_user(response);
    } else {
        Channel* channel = _server.get_channel_by_name(target);
        if (channel == NULL) return _message_to_user(":No such channel", "403");
        if (channel->get_user_in_channel(_user.get_nick()) == NULL)
            return _message_to_user(":You're not on that channel", "442");
        if (message[0] == ':') message.erase(0, 1);
        response = ":" + _user.get_nick() + " PRIVMSG " + channel->get_channel_name() + " :" + message;
        channel->message_to_channel(response, _user.get_user_fd());
    }
}

void Command::_command_part(void) {
    Channel* channel;
    std::string response;
    std::vector<std::string>::iterator it;

    if (_args.size() < 1) return _message_to_user(":Not enough parameters", "461");

    it = _args.begin();
    for (; it != _args.end(); it++) {
        if ((*it)[0] == ':') it->erase(0, 1);
        if ((*it)[0] != '#') *it = '#' + *it;
        channel = _server.get_channel_by_name(*it);
        if (channel == NULL) return _message_to_user(":No such channel", "403");
        if (channel->get_user_in_channel(_user.get_nick()) == NULL)
            return _message_to_user(":You're not on that channel", "442");
        response = ":" + _user.get_nick() + " PART " + *it;
        channel->message_to_channel(response);
        set_next_channel_oper(false, channel->get_channel_name());
        _user.remove_channel(channel->get_channel_name());
        channel->remove_user(&_user);
        _flush_hex(channel->get_channel_name());
    }
}

/**
 * @brief search for next oper in channel if necessary
 *
 * @param shouldUseLoop true: for search in all channel that user are member false: look in one chat in specific
 * @param channel_name if shouldUseLoop false set this for channel_name else do not use
 *
 * @return  a vector string "list" to use for _flush_hex() verify if is empty cause shouldUseLoop does not return elements
 */
std::vector<std::string> Command::set_next_channel_oper(bool shouldUseLoop, std::string channel_name) {
    std::vector<std::string> channels_to_flush;

    if (shouldUseLoop) {
        for (std::map<std::string, bool>::iterator it = _user.user_channel_info.begin();
             it != _user.user_channel_info.end(); it++) {
            std::string channel_name = it->first;
            Channel* channel = _server.get_channel_by_name(channel_name);  // Encontra o canal pelo nome
            if (channel != NULL && channel->get_channel_size() != 1 && it->second) {
                    User* new_user =
                        channel->find_next_channel_oper(_user.get_user_fd());  // Encontra o próximo operador
                    if (new_user != NULL) {
                        new_user->user_channel_info[channel_name] = true;
                        channels_to_flush.push_back(channel_name);
                    }
            }
        }
    } else {
        std::map<std::string, bool>::iterator entry = _user.user_channel_info.find(channel_name);
        if (entry != _user.user_channel_info.end() && entry->second) {
            Channel* channel = _server.get_channel_by_name(channel_name);
            if (channel != NULL && channel->get_channel_size() != 1) {
                User* new_user = channel->find_next_channel_oper(_user.get_user_fd());
                if (new_user != NULL) {
                    new_user->user_channel_info[channel_name] = true;
                }
            }
        }
    }

    return channels_to_flush;
}
