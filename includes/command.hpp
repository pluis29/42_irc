#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include "commonInclude.hpp"

// #define COMMAND_USER 1
// #define ANY_COMMAND 0
// #define OPERATOR_PASS "aprovado"

class Server;
class User;

class Command {
   public:
    Command(std::string buffer, User& user, Server& server);
    ~Command(void);

   private:
    void _parse_buffer(std::string buffer);
    void _handle_command(void);
    void _invalid_command(void);
    void _message_to_user(std::string msg, std::string code, int fd = 0, std::string opt = "");

    User& _user;
    Server& _server;
    std::vector<std::string> _args;
    std::string _command;

    typedef void (Command::*command_handler)(void);
    void _command_pass(void);
    void _command_nick(void);
    void _command_user(void);
};

#endif
