#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include "commonInclude.hpp"

// #define COMMAND_USER 1
// #define ANY_COMMAND 0
#define OPERATOR_PASS "ok"

class Server;
class User;
class Channel;

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

    void _flush_hex(std::string channel_target);
    std::vector<std::string> set_next_channel_oper(bool shouldUseLoop, std::string channel_name = "");
    void _list_server_oper(void);
    void _list_channel_oper(std::string channel_name);

    typedef void (Command::*command_handler)(void);
    void _command_pass(void);
    void _command_nick(void);
    void _command_user(void);
    void _command_quit(void);
    void _command_oper(void);
    void _command_join(void);
    void _command_who(void);
    void _command_privmsg(void);
    void _command_part(void);
    void _command_topic(void);
    void _command_invite(void);
    void _command_mode(void);

    bool mode_need_args(std::string mode);
    void _set_invite_only(Channel* channel, std::string argument);
    void _unset_invite_only(Channel* channel, std::string argument);
    void _set_channel_key(Channel* channel, std::string argument);
    void _unset_channel_key(Channel* channel, std::string argument);
};

#endif
