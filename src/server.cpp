#include "server.hpp"

#include "channel.hpp"
#include "command.hpp"
#include "utils.hpp"

Server::Server(std::string host, std::string port, std::string password)
    : _host(host), _port(port), _password(password) {
    this->set_socket_fd();
    return;
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

    return;
}

void Server::set_socket_fd(void) {
    struct addrinfo *result;
    struct addrinfo *rp;
    struct addrinfo hints;
    int exit_code;
    int server_fd;
    int value = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    exit_code = getaddrinfo(this->_host.c_str(), this->_port.c_str(), &hints, &result);
    if (exit_code != 0) Utils::error_message("getaddrinfo", gai_strerror(exit_code));
    rp = result;
    while (rp) {
        server_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_fd == -1) continue;
        exit_code = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
        if (exit_code != 0) {
            close(server_fd);
            freeaddrinfo(result);
            Utils::error_message("setsockopt", gai_strerror(exit_code));
        }
        exit_code = bind(server_fd, rp->ai_addr, rp->ai_addrlen);
        if (exit_code == 0) break;
        close(server_fd);
        rp = rp->ai_next;
    }
    freeaddrinfo(result);
    if (rp == NULL) Utils::error_message("bind", gai_strerror(exit_code));
    exit_code = listen(server_fd, 50);  // use define
    if (exit_code == -1) Utils::error_message("listen", gai_strerror(exit_code));
    this->_socket_fd = server_fd;
    return;
}

void Server::init_pool(void) {
    struct pollfd pfds = {this->_socket_fd, POLLIN, 0};
    std::vector<pollfd>::iterator it;

    if (fcntl(this->_socket_fd, F_SETFL, O_NONBLOCK) == -1) Utils::error_message("fcntl", strerror(errno));
    this->_pollfd_vector.push_back(pfds);

    while (true) {  // fix
        it = this->_pollfd_vector.begin();
        if (poll(&(*it), this->_pollfd_vector.size(), 5000) == -1) Utils::error_message("poll", strerror(errno));
        this->_handle_polling();
    }
}

void Server::_handle_polling(void) {
    std::vector<pollfd>::iterator it;

    for (it = this->_pollfd_vector.begin(); it != this->_pollfd_vector.end(); it++) {
        if (it->revents && POLLIN) {
            if (it->fd == this->_socket_fd)
                this->_create_user();
            else
                this->_message_recived(it->fd);
            break;
        }
    }
}

void Server::_create_user(void) {
    int user_fd;
    User *new_user;
    struct sockaddr_in cli_addr;
    socklen_t len;

    len = sizeof(cli_addr);
    user_fd = accept(this->_socket_fd, (struct sockaddr *)&cli_addr, &len);
    if (user_fd < 0) Utils::error_message("accept", strerror(errno));
    pollfd user_pfds = {user_fd, POLLIN, 0};
    if (fcntl(user_fd, F_SETFL, O_NONBLOCK) == -1) Utils::error_message("fcntl", strerror(errno));

    new_user = new User(user_fd);
    this->_users_vector.push_back(new_user);
    this->_pollfd_vector.push_back(user_pfds);
    std::cout << "new user " << user_fd << std::endl;
    return;
}

void Server::_message_recived(int fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
    std::string str;

    if (bytesRead <= 0) {
        std::cout << "Client disconnected" << std::endl;
        Command command("/Quit", fd, *this);
        return;
    }
    buffer[bytesRead] = '\0';
    str = buffer;
    std::cout << str << std::endl;
    if (str.find("\n") != std::string::npos) Command command(str, fd, *this);
    str.clear();
}

User *Server::get_user_fd(int user_fd) {
    std::vector<User *>::iterator it = this->_users_vector.begin();

    for (; it != this->_users_vector.end(); it++)
        if ((*it)->get_fd() == user_fd) return *it;
    return NULL;
}

std::string Server::get_password(void) { return this->_password; }

void Server::delete_user(int fd) {
    std::vector<User *>::iterator userIt = this->_users_vector.begin();
    std::vector<pollfd>::iterator pollIt = this->_pollfd_vector.begin();
    std::vector<Channel *>::iterator channelIt = this->_channel_vector.begin();

    for (; channelIt != this->_channel_vector.end(); channelIt++) (*channelIt)->remove_user(get_user_fd(fd));

    for (; pollIt != this->_pollfd_vector.end(); pollIt++) {
        if ((*pollIt).fd == fd) {
            this->_pollfd_vector.erase(pollIt);
            close(fd);
            break;
        }
    }

    for (; userIt != this->_users_vector.end(); userIt++) {
        if ((*userIt)->get_fd() == fd) {
            delete *userIt;
            this->_users_vector.erase(userIt);
            break;
        }
    }
}

std::vector<User *> Server::get_users(void) { return (this->_users_vector); }

// inutil
void Server::message_all_users(std::string msg, int user_fd) {
    std::vector<User *>::iterator it = this->_users_vector.begin();

    if (msg.find("\r\n") == std::string::npos) msg += "\r\n";

    for (; it != this->_users_vector.end(); it++)
        if ((*it)->get_fd() != user_fd)
            if (send((*it)->get_fd(), msg.c_str(), strlen(msg.c_str()), 0) < 0)
                Utils::error_message("messageToServer: send:", strerror(errno));

    return;
}

bool Server::check_operators(void) {
    std::vector<User *>::iterator it = this->_users_vector.begin();
    for (; it != this->_users_vector.end(); it++)
        if ((*it)->is_oper()) return true;

    return false;
}

User *Server::find_next_oper(int user_fd) {
    std::vector<User *>::iterator userIt = this->_users_vector.begin();
    for (; userIt != this->_users_vector.end(); userIt++) {
        if (!((*userIt)->get_username().empty()) && (*userIt)->get_fd() != user_fd) return *userIt;
    }
    return NULL;
}

User *Server::get_user_byNick(std::string nick) {
    std::vector<User *>::iterator it = this->_users_vector.begin();

    for (; it != this->_users_vector.end(); it++)
        if ((*it)->get_nick() == nick) return *it;
    return NULL;
}

Channel *Server::get_channel(std::string name) {
    std::vector<Channel *>::iterator it = this->_channel_vector.begin();

    if (name[0] != '#') name = '#' + name;

    for (; it != this->_channel_vector.end(); it++) {
        if ((*it)->get_name() == name) return (*it);
    }
    return NULL;
}

void Server::add_channel(Channel *channel) { this->_channel_vector.push_back(channel); }
