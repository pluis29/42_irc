#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

#include "commonInclude.hpp"

class User;

class Channel {
   public:
    Channel(std::string name, std::string password);
    ~Channel(void);
    std::string get_channel_name(void) const;
    User* get_user_in_channel(std::string nick);
    std::string get_password(void);
    void add_user(User* user);
    /* std::string get_name(void); */
    void message_to_channel(std::string msg);
    void message_to_channel(std::string msg, int exclude_fd);
    /* std::vector<User*> get_users(void); */
    void remove_user(User* user);
    std::vector<User*> get_user_list(void) const;

    bool find_channel_oper(void);
    bool find_user_in_channel(int user_fd);

    User* find_next_channel_oper(int user_fd);
    int get_channel_size() const;

    std::string get_topic(void) const;
    void clear_topic(void);
    void set_topic(std::string topic);

    bool is_invite_only(void);
    bool is_topic_restricted(void);
    bool is_user_limit(void);
    bool invite_only;
    bool topic_restrict;
    bool have_user_limit;
    int user_limit;
    std::string password;

   private:
    std::string _name;
    std::string _topic;
    std::vector<User*> _users;
};

#endif
