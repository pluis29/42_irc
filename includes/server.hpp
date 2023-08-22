#ifndef SERVER_HPP_
#define SERVER_HPP_

#include "commonInclude.hpp"

// #define SERVER ":ft_irc "
#define SERVERNAME "ft_irc"
#define BUFFER_SIZE 1024
#define HOST_IP "127.0.0.1"  // talvez remover
#define MAX_CONNECTION 50

class User;

class Server {
   public:
    Server(const int port, const std::string password);
    ~Server(void);
    void run(void);
    void message_all_users(std::string msg, int user_fd);

    User *get_user_by_fd(int user_fd);
    std::string get_password(void) const;
    std::vector<User *> get_users_in_server(void) const;

   private:
    const int _port;
    const std::string _password;
    const std::string _host_ip;

    std::vector<pollfd> _pollfd_vector;
    std::vector<User *> _users_vector;

    int _server_socket;

    void _message_recived(int fd);
    void _set_server_socket(void);
    void _handle_polling(void);
    void _create_user(void);
};

#endif  // SERVER_HPP_
