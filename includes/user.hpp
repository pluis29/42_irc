#ifndef USER_HPP_
#define USER_HPP_

#include "commonInclude.hpp"

class Channel;

class User {
   public:
    User(int user_fd);
    ~User(void);

    bool is_general_auth(void) const;
    bool is_password_auth(void) const;
    bool is_server_oper(void) const;
    bool is_oper_in_channel(const std::string& channel_name);
    bool is_invited_to_channel(std::string channel_name);

    void set_hostname(std::string user_ip);
    void set_password_auth(void);
    void set_nick(std::string nick);
    void set_general_auth(void);
    void set_username(std::string username);
    void set_realname(std::string realname);
    void set_servername(std::string servername);
    void set_server_oper(void);

    int get_user_fd(void) const;
    std::string get_hostname(void) const;
    std::string get_nick(void) const;
    std::string get_username(void) const;
    std::string get_servername(void) const;

    void send_message_to_user(std::string message);
    void add_channel(Channel* channel);
    void remove_channel(std::string channel_name);

    std::map<std::string, bool> user_channel_info;
    std::vector<std::string> channel_invites;

   private:
    int _user_fd;
    std::string _nick;
    std::string _username;
    bool _general_auth;
    bool _password_auth;
    bool _server_oper;
    std::string _realname;
    std::string _servername;
    std::string _hostname;
};

#endif  // USER_HPP_
