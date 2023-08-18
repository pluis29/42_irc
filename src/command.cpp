#include "command.hpp"

#include <cstddef>
#include <ostream>

#include "utils.hpp"

Command::Command(std::string buffer, int user_fd, Server& server)
    : _user(*server.get_user_fd(user_fd)), _server(server) {
    _parse_buffer(buffer);
    _handle_command();
}

Command::~Command(void) { return; }

void Command::_parse_buffer(std::string buffer) {
    if (buffer[0] == '/') buffer.erase(0, 1);
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'), buffer.end());

    this->_args = Utils::split(buffer, ' ');
    if (this->_args.size() == 0) return;

    this->_command = Utils::toUpperCase(*this->_args.begin());
    /* std::cout << this->_command << std::endl; */
    this->_args.erase(this->_args.begin());
    if (this->_args[0][0] == ':') this->_args[0].erase(0, 1);
    return;
}

void Command::_handle_command(void) {
    std::map<std::string, command_handler> command_map;
    std::map<std::string, command_handler> auth_command_map;

    command_map["PASS"] = &Command::_command_pass;
    command_map["QUIT"] = &Command::_command_quit;
    auth_command_map["NICK"] = &Command::_command_nick;
    auth_command_map["USER"] = &Command::_command_user;
    auth_command_map["OPER"] = &Command::_command_oper;

    std::map<std::string, command_handler>::iterator it = command_map.find(this->_command);
    std::map<std::string, command_handler>::iterator it2 = auth_command_map.find(this->_command);

    if (it != command_map.end()) {
        (this->*(it->second))();
        return;
    }
    if (this->_user.check_auth() && it2 != auth_command_map.end())
        (this->*(it2->second))();
    else
        return this->_invalid_command();
}

void Command::_invalid_command(void) {
    std::string response;

    if (!this->_user.check_auth()) {
        message_to_user("Authenticate is required: usage: /PASS <password>", "464");
        return;
    } else if (this->_user.get_nick().empty()) {
        message_to_user("A nick must be provide: usage: /NICK <nick>", "431");
        return;
    } else if (this->_user.get_username().empty()) {
        message_to_user("A user must be provide: usage: /USER <username> <hostname> "
                        "<servername> <realname>",
                        "431");
        return;
    }
    response = ":127.0.0.1 421 " + this->_user.get_nick() + " " + this->_command + " :Unknown command\r\n";
    if (send(this->_user.get_fd(), response.c_str(), strlen(response.c_str()), 0) < 0)
        Utils::error_message("send:", strerror(errno));
}

void Command::_command_nick(void) {
    std::vector<User*> users;
    std::vector<User*>::iterator it;

    if (this->_args.size() != 1) return (message_to_user(":No nickname given", "431"));
    if (this->_args[0].empty() || Utils::check_invalid_char(this->_args[0]))
        return (message_to_user(":Erroneous nickname", "432"));
    users = this->_server.get_users();
    it = users.begin();
    for (; it != users.end(); it++) {
        if (this->_args[0] == (*it)->get_nick()) return (message_to_user(":Nickname is already in use", "433"));
    }
    this->_user.set_nick(this->_args[0]);
    message_to_user("change nick to " + this->_user.get_nick(), "001");
}

void Command::_command_pass(void) {
    if (this->_user.check_auth()) return (message_to_user(":Unauthorized command (already registered)", "462"));
    if (this->_args.empty() || this->_args.size() != 1) return (message_to_user(":Not enough parameters", "461"));
    if (this->_args[0] == this->_server.get_password()) {
        this->_user.set_auth();
        return (message_to_user(":Password Correct", "001"));
    } else
        return (message_to_user(":Password incorrect", "339"));
}

void Command::_command_oper(void) {
    User* user;

    if (!this->_check_user_registration(0)) return;
    if (this->_args.size() != 2) return (message_to_user(":Not enough parameters", "461"));
    user = this->_server.get_user_byNick(this->_args[0]);
    if (user == NULL ) return (message_to_user(":No such nick", "401"));
    if (user->is_oper()) return (message_to_user(":You are already an operator.", "690"));
    if (this->_args[1] == OPERATOR_PASS)
        user->set_operator();
    else
        return (message_to_user(":Password incorrect", "464"));
    message_to_user(":You are now an IRC operator", "381", user->get_fd());
    return;
}

void Command::_command_user(void) {
    if (!this->_check_user_registration(COMMAND_USER)) return;
    if (this->_args.size() != 4) return (message_to_user(":Not enough parameters", "461"));
    if (this->_user.get_username().empty() == false)
        return (message_to_user(":Unauthorized command (already registered)", "462"));
    this->_user.set_username(this->_args[0]);
    this->_user.set_hostname(this->_args[1]);
    this->_user.set_servername(this->_args[2]);
    this->_user.set_realname(this->_args[3]);
    message_to_user("Welcome to the ft_irc " + this->_user.get_nick() + "!" + this->_user.get_username() + "@" +
                        this->_user.get_hostname(),
                    "001");

    if (!this->_server.check_operators()) {
        this->_user.set_operator();
        message_to_user(":You are now an IRC operator", "381");
    }
    return;
}

/**
 * @brief for commands Authenticate check if others methods if already
 * Authenticate
 *
 * @param flag = 1 (COMMAND_USER); flag = 0 ANY_COMMAND
 */
bool Command::_check_user_registration(int flag) {
    if (this->_user.get_nick().empty()) {
        message_to_user("A nick must be provide: usage: /NICK <nick>", "431");
        return false;
    }
    if (flag == COMMAND_USER)  // caso username function
        return true;
    if (this->_user.get_username().empty()) {
        message_to_user("A user must be provide: usage: /USER <username> <hostname> "
                        "<servername> <realname>",
                        "431");
        return false;
    }
    return true;
}

/* // otimizar isso */
void Command::message_to_user(std::string msg, std::string code, int fd, std::string opt) {
    std::string response;
    std::string nick = this->_user.get_nick();

    if (nick.empty()) nick = "Unknown";
    response = SERVER + code + " " + nick + " ";
    if (opt != "") response += opt + " ";
    response += msg + "\r\n";
    if (fd == 0) fd = this->_user.get_fd();
    if (send(fd, response.c_str(), strlen(response.c_str()), 0) < 0) Utils::error_message("send:", strerror(errno));

    return;
}

void Command::_command_quit(void) {
    User* user;
    std::string response;

    /* response = ":" + this->_user.get_nick() + " QUIT :" + */
    /*            Utils::join_split(this->_args.begin() + 1, this->_args.end());
     */

    /* this->_server.message_all_users(response); */
    if (this->_user.is_oper()) this->_user.set_operator();

    if (this->_server.get_users().size() != 1 && this->_server.check_operators() == false) {
        user = this->_server.find_next_oper(this->_user.get_fd());
        if (user != NULL) {
            user->is_oper();
            message_to_user(":You are now an IRC operator", "381", user->get_fd());
        }
    }
    std::cout << "User left. FD: " << this->_user.get_fd() << std::endl;
    this->_server.delete_user(this->_user.get_fd());
}
