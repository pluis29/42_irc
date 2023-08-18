#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include <algorithm>
#include <map>
#include <cstddef>
#include <ostream>

#include "server.hpp"
#include "user.hpp"
#include "utils.hpp"
// devo incluir as libs necessarias ou usar as que ja estao nos headers?

#define COMMAND_USER 1
#define ANY_COMMAND 0
#define OPERATOR_PASS "aprovado"

class Command {
   private:
    User &_user;
    Server &_server;
    std::vector<std::string> _args;
    std::string _command;

    void _parse_buffer(std::string buffer);
    void _handle_command(void);
    bool _check_user_registration(int flag);
    void _invalid_command(void);

    typedef void (Command::*command_handler)(void);
    void _command_pass(void);
    void _command_quit(void);
    void _command_nick(void);
    void _command_user(void);
    void _command_oper(void);
    void _command_kill(void);

   public:
    void message_to_user(std::string msg, std::string code, int fd = 0, std::string opt = "");
    Command(std::string buffer, int user_fd, Server &server);
    ~Command(void);
};

#endif
