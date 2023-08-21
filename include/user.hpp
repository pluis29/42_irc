#ifndef USER_HPP_
#define USER_HPP_


#include "common.hpp"

class Channel;

class User {
   public:
    User(int user_fd);
    ~User(void);
    int get_fd(void);
    std::string get_nick(void);
    int check_auth(void);
    void set_auth(void);
    std::string get_username(void);
    void set_nick(std::string nick);
    void set_username(std::string username);
    void set_hostname(std::string hostname);
    void set_realname(std::string realname);
    void set_servername(std::string servername);
    std::string get_hostname(void);
    void set_operator(void);
    bool is_oper(void);
    void send_message_to_user(std::string message);
    void add_channel(Channel* channel);
    std::string get_realname(void);




    std::string get_servername(void);

   private:
    int _user_fd;
    std::string _nick;
    std::string _username;
    std::string _realname;
    std::string _servername;
    std::string _hostname;
    bool _auth;
    bool _oper;
    std::vector<Channel*> _channel_vector;
};

#endif
