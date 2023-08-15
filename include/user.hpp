#ifndef USER_HPP_
#define USER_HPP_

#include <unistd.h>

#include <iostream>

class User {
   public:
    User(int user_fd);
    ~User(void);
    int get_fd(void);
    std::string get_nick(void);
    int check_auth(void);
    void set_auth(void);
    std::string get_username(void);

   private:
    int _user_fd;
    std::string _nick;
    std::string _username;
    bool _auth;
    bool _oper;
};

#endif
