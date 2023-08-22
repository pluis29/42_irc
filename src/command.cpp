#include "command.hpp"

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

// user vai ter flag
void Command::_handle_command(void) {
    std::map<std::string, command_handler> command_map;

    command_map["PASS"] = &Command::_command_pass;  // --
    command_map["NICK"] = &Command::_command_nick;  // --
    command_map["USER"] = &Command::_command_user;  // --
    /* command_map["QUIT"] = &Command::_command_quit; */
    if (_user.is_general_auth()) {
        // command_map["OPER"] = &Command::_command_oper;  //--
        // command_map["KILL"] = &Command::_command_kill;  //--
        // command_map["JOIN"] = &Command::_command_join;
        // command_map["WHO"] = &Command::_command_who;  //--
        // auth_command_map["PRIVMSG"] = &Command::_command_privmsg;  //--
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

    // if (!this->_server.check_operators()) {
    //     this->_user.set_operator();
    //     message_to_user(":You are now an IRC operator", "381");
    // }
}
