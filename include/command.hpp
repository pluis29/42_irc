#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include "common.hpp"

#define COMMAND_USER 1
#define ANY_COMMAND 0
#define OPERATOR_PASS "aprovado"

class Server;
class User;

class Command {
   public:
    void message_to_user(std::string msg, std::string code, int fd = 0, std::string opt = "");
    Command(std::string buffer, int user_fd, Server &server);
    ~Command(void);

   private:
    User &_user;
    Server &_server;
    std::vector<std::string> _args;
    std::string _command;

    void _parse_buffer(std::string buffer);
    void _handle_command(void);
    bool _check_user_registration(int flag);
    void _invalid_command(void);

    void _flush_hex(std::string channel_name);
    void _give_oper_to_creator(std::string channel_name);

    typedef void (Command::*command_handler)(void);
    void _command_pass(void);
    void _command_quit(void);
    void _command_nick(void);
    void _command_user(void);
    void _command_oper(void);
    void _command_kill(void);
    void _command_join(void);
    void _command_who(void);
    void _command_privmsg(void);
};

#endif
