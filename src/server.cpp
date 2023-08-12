#include "server.hpp"

#include "utils.hpp"

Server::Server(std::string host, std::string port, std::string password)
    : _host(host), _port(port), _password(password) {
    this->set_socket_fd();
    return;
}

void Server::set_socket_fd(void) {
    struct addrinfo *result, *rp;
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
        Utils::errorMessage("getaddrinfo", gai_strerror(exit_code));
    rp = result;
    while (rp) {
        server_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_fd == -1) continue;
        exit_code = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &value,
                               sizeof(value));
        if (exit_code != 0) {
            close(server_fd);
            freeaddrinfo(result);
            Utils::errorMessage("setsockopt", gai_strerror(exit_code));
        }
        exit_code = bind(server_fd, rp->ai_addr, rp->ai_addrlen);
        if (exit_code == 0) break;
        close(server_fd);
        rp = rp->ai_next;
    }
    freeaddrinfo(result);
    if (rp == NULL) Utils::errorMessage("bind", gai_strerror(exit_code));
    exit_code = listen(server_fd, 50);  // use define
    if (exit_code == -1)
        Utils::errorMessage("listen:", gai_strerror(exit_code));
    this->_socket_fd = server_fd;
}
