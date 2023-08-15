#include "server.hpp"

#include "utils.hpp"

Server::Server(std::string host, std::string port, std::string password)
    : _host(host), _port(port), _password(password) {
    this->set_socket_fd();
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
    exit_code =
        getaddrinfo(this->_host.c_str(), this->_port.c_str(), &hints, &result);
    if (exit_code != 0)
        Utils::error_message("getaddrinfo", gai_strerror(exit_code));
    rp = result;
    while (rp) {
        server_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_fd == -1) continue;
        exit_code = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &value,
                               sizeof(value));
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
    if (exit_code == -1)
        Utils::error_message("listen", gai_strerror(exit_code));
    this->_socket_fd = server_fd;
    return;
}

void Server::init_pool(void) {
    struct pollfd pfds = {this->_socket_fd, POLLIN, 0};
    std::vector<pollfd>::iterator it;

    if (fcntl(this->_socket_fd, F_SETFL, O_NONBLOCK) == -1)
        Utils::error_message("fcntl", strerror(errno));
    this->_pollfd_vector.push_back(pfds);

    while (true) {  // fix
        it = this->_pollfd_vector.begin();
        if (poll(&(*it), this->_pollfd_vector.size(), 5000) == -1)
            Utils::error_message("poll", strerror(errno));
        this->_handle_polling();
    }
}

void Server::_handle_polling(void) {
    std::vector<pollfd>::iterator it;

    for (it = this->_pollfd_vector.begin(); it != this->_pollfd_vector.end();
         it++) {
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
    if (fcntl(user_fd, F_SETFL, O_NONBLOCK) == -1)
        Utils::error_message("fcntl", strerror(errno));

    new_user = new User(user_fd);
    this->_users_vector.push_back(new_user);
    this->_pollfd_vector.push_back(user_pfds);
    std::cout << "new user " << user_fd << std::endl;
    return;
}

// otimizar
void Server::_message_recived(int fd) {
    char buff;
    std::string str;
    int a = 0;

    while (str.find("\n")) {
        if (recv(fd, &buff, 1, 0) < 0) {
            continue;
        } else {
            str += buff;
            if (a > 500) str = "/Quit not today!\r\n";

            if (str.find("\n") != std::string::npos) {
                if (str.size() == 1) str = "/Quit not today!\r\n";
                std::cout << "fd: " << fd << "input >> " << str << std::endl;
                Command command(str, fd, *this);
                break;
            }
        }
        a++;
    }
    str.clear();
}

User *Server::get_user_fd(int user_fd) {
    std::vector<User *>::iterator it = this->_users_vector.begin();

    for (; it != this->_users_vector.end(); it++)
        if ((*it)->get_fd() == user_fd) return *it;
    return NULL;
}

std::string Server::get_password(void) { return this->_password; }
