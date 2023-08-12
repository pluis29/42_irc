#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>

#define HOST "127.0.0.1"

class Server {
   public:
    Server(std::string host, std::string port, std::string password);
    void set_socket_fd(void);

   private:
    std::string _host;
    std::string _port;
    std::string _password;
    int _socket_fd;
};

#endif  // SERVER_HPP_
