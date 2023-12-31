#ifndef SERVER_HPP_
#define SERVER_HPP_

#include "commonInclude.hpp"

#define SERVERNAME "ft_irc"
#define BUFFER_SIZE 1024
#define HOST_IP "127.0.0.1"
#define MAX_CONNECTION 50

class User;
class Channel;

class Server {
   public:
    Server(const int port, const std::string password);
    ~Server(void);

    void run(void);
    void message_all_users(std::string msg, int user_fd);
    void add_channel(Channel *channel);
    void delete_user(int fd);

    User *get_user_by_fd(int user_fd);
    std::string get_password(void) const;
    std::vector<User *> get_users_in_server(void) const;
    User *get_user_by_nick(std::string nick);
    Channel *get_channel_by_name(std::string name);

    bool find_server_oper(void);
    User *find_next_server_oper(int user_fd);

   private:
    const std::string _host_ip;
    const int _port;
    const std::string _password;
    std::vector<pollfd> _pollfd_vector;
    std::vector<User *> _users_vector;
    std::vector<Channel *> _channel_vector;
    int _server_socket;

    void _message_recived(int fd);
    void _handle_polling(void);
    void _create_user(void);
    void _set_server_socket(void);

    std::map<int, std::string> _str_command;
};

#endif  // SERVER_HPP_
