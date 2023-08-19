#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

#include "common.hpp"

class User;

class Channel {
   public:
    Channel(std::string name, std::string password);
    ~Channel(void);
    std::string get_name(void);
    User* get_user_in_channel(std::string nick);
    std::string get_password(void);
    void add_user(User* user);
    void message_to_channel(std::string msg);
    std::vector<User*> get_users(void);
    void remove_user(User* user);

   private:
    std::string _name;
    std::string _password;
    std::vector<User*> _users;
};

#endif
