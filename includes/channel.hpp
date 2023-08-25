#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

#include "commonInclude.hpp"

class User;

class Channel {
   public:
    Channel(std::string name, std::string password);
    ~Channel(void);

    void add_user(User* user);
    void message_to_channel(std::string msg);
    void message_to_channel(std::string msg, int exclude_fd);
    void remove_user(User* user);
    void clear_topic(void);

    bool find_channel_oper(void);
    bool find_user_in_channel(int user_fd);
    User* find_next_channel_oper(int user_fd);

    std::string get_channel_name(void) const;
    User* get_user_in_channel(std::string nick);
    std::string get_password(void);
    std::vector<User*> get_user_list(void) const;
    int get_channel_size() const;
    std::string get_topic(void) const;
    void set_topic(std::string topic);

    bool is_invite_only(void);
    bool is_topic_restricted(void);
    bool is_user_limit(void);

    std::string name;
    std::string password;
    bool topic_restrict;
    bool invite_only;
    bool have_user_limit;
    int user_limit;

   private:
    std::string _topic;
    std::vector<User*> _users;
};

#endif  // CHANNEL_HPP_
