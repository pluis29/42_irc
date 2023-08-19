#ifndef SERVER_HPP_
#define SERVER_HPP_

#include "common.hpp"

#define HOST "127.0.0.1"
#define SERVER ":ft_irc "
#define BUFFER_SIZE 1024

class Channel;
class User;

class Server {
   public:
    Server(std::string host, std::string port, std::string password);
    ~Server(void);
    void set_socket_fd(void);
    void init_pool(void);
    User *get_user_fd(int user_fd);
    std::string get_password(void);
    void delete_user(int fd);
    std::vector<User *> get_users(void);
    void message_all_users(std::string msg, int user_fd);
    bool check_operators(void);
    User *find_next_oper(int user_fd);
    User *get_user_byNick(std::string nick);
    Channel *get_channel(std::string name);
    void add_channel(Channel *channel);

   private:
    std::string _host;
    std::string _port;
    std::string _password;
    int _socket_fd;
    std::vector<pollfd> _pollfd_vector;
    std::vector<User *> _users_vector;
    std::vector<Channel *> _channel_vector;

    void _handle_polling(void);
    void _create_user(void);
    void _message_recived(int fd);
};

#endif  // SERVER_HPP_
