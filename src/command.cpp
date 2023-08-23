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
        // command_map["WHO"] = &Command::_command_who;  //--
        // auth_command_map["PRIVMSG"] = &Command::_command_privmsg;  //--
        // command_map["KILL"] = &Command::_command_kill;  //--
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
    if (!_user.is_password_auth() && _user.get_nick().empty()) return _invalid_command();

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

// quando um server oper sair mandar oper para o prox ou se ele for channel oper mandar para o prox isso depende do novo
// vector de user e operchannel so quando tiver fazer isso e otimizar o movimento de opers para o sucessores
// para deletar o user preciso tbm saber em qual array ele esta se e na oper ou na user
void Command::_command_quit(void) {
    /* std::string response; */
    /* response = ":" + this->_user.get_nick() + " QUIT :" + */
    /*            Utils::join_split(this->_args.begin() + 1, this->_args.end());
     */
    /* this->_server.message_all_users(response); */

    if (this->_user.is_server_oper()) this->_user.set_server_oper();

    if (this->_server.get_users_in_server().size() != 1 && !this->_server.find_server_oper()) {
        User* user;
        user = this->_server.find_next_server_oper(this->_user.get_user_fd());
        if (user != NULL) {
            user->set_server_oper();
            _message_to_user(":You are now an IRC operator", "381", user->get_user_fd());
        }
    }
    /* for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) { */
    /*     if ((*it)->get_size() != 1) { */
    /*         User* user = (*it)->get_user_channel(username); */
    /*         if (user != NULL) { */
    /*             if (user->is_oper_in_channel()) { */
    /*                 User* new_user = find_next_oper(user); // Encontrar próximo operador */
    /*                 if (new_user != NULL) { */
    /*                     new_user->set_oper_in_channel(true); // Configurar new_user como operador no canal */
    /*                     new_user.oper_map[(*it)->get_name_channel] = true; */
    /*                     _user.oper_map.erase((*it)->get_name_channel()); */
    /*             } */
    /*         } */
    /*     } */
    /* } */

    std::cout << "User left. FD: " << this->_user.get_user_fd() << std::endl;
    this->_server.delete_user(this->_user.get_user_fd());
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
