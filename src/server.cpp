#include "server.hpp"

#include "channel.hpp"
#include "command.hpp"
#include "user.hpp"
#include "utils.hpp"

Server::Server(const int port, const std::string password) : _host_ip(HOST_IP), _port(port), _password(password) {
    _set_server_socket();
}

Server::~Server(void) {
    std::vector<User *>::iterator userIt = this->_users_vector.begin();
    std::vector<pollfd>::iterator pollIt = this->_pollfd_vector.begin();
    std::vector<Channel *>::iterator channelIt = this->_channel_vector.begin();

    for (; channelIt != this->_channel_vector.end(); channelIt++) {
        delete *channelIt;
    }
    this->_channel_vector.clear();

    for (; pollIt != this->_pollfd_vector.end(); pollIt++) {
        close((*pollIt).fd);
    }

    for (; userIt != this->_users_vector.end(); userIt++) {
        delete *userIt;
    }
    this->_users_vector.clear();
}

void Server::_set_server_socket(void) {
    int exit_code;
    int value = 1;

    _server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_socket == -1) {
        Utils::error_message("socket:", strerror(errno));
    }
    exit_code = setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
    if (exit_code < 0) {
        close(_server_socket);
        Utils::error_message("setsockopt", gai_strerror(exit_code));
    }
    if (fcntl(_server_socket, F_SETFL, O_NONBLOCK) == -1) {
        close(_server_socket);
        Utils::error_message("fcntl:", strerror(errno));
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_port);

    exit_code = bind(_server_socket, (sockaddr *)&serverAddr, sizeof(serverAddr));
    if (exit_code == -1) {
        close(_server_socket);
        Utils::error_message("bind:", strerror(errno));
    }
    exit_code = listen(_server_socket, MAX_CONNECTION);
    if (exit_code == -1) {
        close(_server_socket);
        Utils::error_message("listen:", strerror(errno));
    }
}

void Server::run(void) {
    struct pollfd pfds = {_server_socket, POLLIN, 0};

    this->_pollfd_vector.push_back(pfds);
    while (true) {
        if (poll(&this->_pollfd_vector[0], this->_pollfd_vector.size(), 1) == -1) {
            Utils::error_message("poll", strerror(errno));
        }
        _handle_polling();
    }
}

void Server::_handle_polling(void) {
    for (std::vector<pollfd>::iterator it = _pollfd_vector.begin(); it != _pollfd_vector.end(); it++) {
        if (it->revents && POLLIN) {
            if (it->fd == _server_socket) {
                _create_user();

            } else {
                _message_recived(it->fd);
            }
            break;
        }
    }
}

void Server::_create_user(void) {
    int user_fd;
    User *new_user;
    struct sockaddr_in user_addr;
    socklen_t user_len = sizeof(user_addr);

    user_fd = accept(_server_socket, (struct sockaddr *)&user_addr, &user_len);
    if (user_fd < 0) {
        Utils::error_message("accept", strerror(errno));
    }
    if (fcntl(user_fd, F_SETFL, O_NONBLOCK) == -1) {
        Utils::error_message("fcntl", strerror(errno));
    }
    pollfd user_pfds = {user_fd, POLLIN, 0};

    new_user = new User(user_fd);
    _users_vector.push_back(new_user);
    _pollfd_vector.push_back(user_pfds);
    new_user->set_hostname(inet_ntoa(user_addr.sin_addr));
    new_user->set_servername(SERVERNAME);

    std::cout << "New user connection socket_fd: " << user_fd << std::endl;
}

void Server::_message_recived(int fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        std::cout << "Client disconnected fd: " << fd << std::endl;
        Command command("/Quit", *get_user_by_fd(fd), *this);
        return;
    }

    buffer[bytesRead] = '\0';
    std::string str = buffer;
    std::cout << "User:" << fd << " --> " << str << std::endl;
    if (str.find("\n") != std::string::npos) {
        Command command(str, *get_user_by_fd(fd), *this);
    }
    str.clear();
}

void Server::message_all_users(std::string msg, int user_fd) {
    std::vector<User *>::iterator it = this->_users_vector.begin();

    if (msg.find("\r\n") == std::string::npos) {
        msg += "\r\n";
    }

    for (; it != this->_users_vector.end(); it++) {
        if ((*it)->get_user_fd() != user_fd) {
            if (send((*it)->get_user_fd(), msg.c_str(), msg.size(), 0) < 0) {
                Utils::error_message("messageToServer: send:", strerror(errno));
            }
        }
    }
}

bool Server::find_server_oper(void) {
    std::vector<User *>::iterator it = _users_vector.begin();
    for (; it != _users_vector.end(); it++) {
        if ((*it)->is_server_oper()) {
            return true;
        }
    }

    return false;
}

User *Server::find_next_server_oper(int user_fd) {
    std::vector<User *>::iterator userIt = this->_users_vector.begin();
    for (; userIt != this->_users_vector.end(); userIt++) {
        if (!((*userIt)->get_username().empty()) && (*userIt)->get_user_fd() != user_fd) {
            return *userIt;
        }
    }
    return NULL;
}

void Server::delete_user(int fd) {
    std::vector<User *>::iterator userIt = this->_users_vector.begin();
    std::vector<pollfd>::iterator pollIt = this->_pollfd_vector.begin();
    std::vector<Channel *>::iterator channelIt = this->_channel_vector.begin();

    for (; channelIt != this->_channel_vector.end(); channelIt++) {
        (*channelIt)->remove_user(get_user_by_fd(fd));
    }

    for (; pollIt != this->_pollfd_vector.end(); pollIt++) {
        if ((*pollIt).fd == fd) {
            this->_pollfd_vector.erase(pollIt);
            close(fd);
            break;
        }
    }

    for (; userIt != this->_users_vector.end(); userIt++) {
        if ((*userIt)->get_user_fd() == fd) {
            delete *userIt;
            this->_users_vector.erase(userIt);
            break;
        }
    }
}

void Server::add_channel(Channel *channel) { this->_channel_vector.push_back(channel); }

std::string Server::get_password(void) const { return _password; }

std::vector<User *> Server::get_users_in_server(void) const { return _users_vector; }

User *Server::get_user_by_nick(std::string nick) {
    std::vector<User *>::iterator it = this->_users_vector.begin();

    for (; it != this->_users_vector.end(); it++) {
        if ((*it)->get_nick() == nick) {
            return *it;
        }
    }
    return NULL;
}

Channel *Server::get_channel_by_name(std::string name) {
    std::vector<Channel *>::iterator it = _channel_vector.begin();

    if (name[0] != '#') {
        name = '#' + name;
    }
    for (; it != _channel_vector.end(); it++) {
        if ((*it)->get_channel_name() == name) {
            return (*it);
        }
    }
    return NULL;
}

User *Server::get_user_by_fd(int user_fd) {
    std::vector<User *>::iterator it = this->_users_vector.begin();

    for (; it != this->_users_vector.end(); it++) {
        if ((*it)->get_user_fd() == user_fd) {
            return *it;
        }
    }
    return NULL;
}
