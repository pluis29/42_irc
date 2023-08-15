#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include <algorithm>
#include <map>

#include "server.hpp"
#include "user.hpp"
#include "utils.hpp"
// devo incluir as libs necessarias ou usar as que ja estao nos headers?
//
class Server;

class Command {
   private:
    User &_user;
    Server &_server;
    std::vector<std::string> _args;
    std::string _command;

    void _parse_buffer(std::string buffer);
    void _handle_command(void);
    int _check_user_registration(void);
    void _invalid_command(void);

    typedef void (Command::*command_handler)(void);
    void _command_pass(void);

   public:
    void message_to_user(std::string msg, std::string code, int fd = 0,
            std::string opt = "");
    Command(std::string buffer, int user_fd, Server &server);
    ~Command(void);
};

#endif
