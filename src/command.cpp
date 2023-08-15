#include "command.hpp"

#include <ostream>

#include "utils.hpp"

Command::Command(std::string buffer, int user_fd, Server &server)
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
    std::cout << this->_command << std::endl;
    this->_args.erase(this->_args.begin());
    if (this->_args[0][0] == ':') this->_args[0].erase(0, 1);
    return;
}

void Command::_handle_command(void) {
    std::map<std::string, command_handler> command_map;

    command_map["PASS"] = &Command::_command_pass;

    std::map<std::string, command_handler>::iterator it =
        command_map.find(this->_command);
    if (it != command_map.end())
        (this->*(it->second))();
    else
        return this->_invalid_command();
    return;
}

void Command::_invalid_command(void) {
    std::string response;

    if (!this->_user.check_auth()) {
        message_to_user("Password was required: usage: /PASS <password>",
                        "464");
        return;
    } else if (this->_user.get_nick().empty()) {
        message_to_user("A nick must be provide: usage: /NICK <nick>", "431");
        return;
    } else if (this->_user.get_username().empty()) {
        message_to_user(
            "A user must be provide: usage: /USER <username> <hostname> "
            "<servername> <realname>",
            "431");
        return;
    }
    response = ":127.0.0.1 421 " + this->_user.get_nick() + " " +
               this->_command + " :Unknown command\r\n";
    if (send(this->_user.get_fd(), response.c_str(), strlen(response.c_str()),
             0) < 0)
        Utils::error_message("send:", strerror(errno));
}

void Command::_command_pass(void) {
    if (this->_user.check_auth())
        return (message_to_user("User already registered", "462"));
    if (this->_args.empty() || this->_args.size() != 1)
        return (message_to_user("usage: /PASS <password>", "461"));
    if (this->_args[0] == this->_server.get_password()) {
        this->_user.set_auth();
        return (message_to_user("Password Correct", "338"));
    } else
        return (message_to_user("Incorrect Password", "339"));
}

/* // voltar aqui para quando o user executar algo que precise estar auth */
/* int Command::_check_user_registration(void) { */
/*     if (!this->_user.isAuth()) { */
/*         message_to_user("Password was required: usage: /PASS <password>", */
/*                         "464"); */
/*         return false; */
/*     } */
/*     // nick e user /... */
/*     return true; */
/* } */

/* // otimizar isso */
void Command::message_to_user(std::string msg, std::string code, int fd,
                              std::string opt) {
    std::string response;
    std::string nick = this->_user.get_nick();

    if (nick.empty()) nick = "Unknown";
    response = ":127.0.0.1 " + code + " " + nick + " ";
    if (opt != "") response += opt + " ";
    response += msg + "\r\n";
    if (fd == 0) fd = this->_user.get_fd();
    if (send(fd, response.c_str(), strlen(response.c_str()), 0) < 0)
        Utils::error_message("send:", strerror(errno));

    return;
}
